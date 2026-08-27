/**
 * @file PenguinStateMachine.cpp
 * @brief ペンギンのステートマシン
 */
#include "stdafx.h"
#include "PenguinBase.h"
#include "PenguinIState.h"
#include "PenguinStateMachine.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			constexpr float JUMP_STAMINA_MAX = 100.0f;
			constexpr float JUMP_STAMINA_DECREASE_SPEED = 0.0f;
			constexpr float JUMP_STAMINA_RECOVER_SPEED = 50.0f;

			constexpr float SLIDE_STAMINA_MAX = 100.0f;
			constexpr float SLIDE_STAMINA_DECREASE_SPEED = 50.0f;
			constexpr float SLIDE_STAMINA_RECOVER_SPEED = 25.0f;


			//============================================//
			// スライドの傾斜モデル
			// 各定数の根拠と数値表は docs/スライドの傾斜モデル.md にある。
			// 触るときは必ずそちらの表も更新すること。
			//============================================//

			/**
			 * @brief 下りの効き
			 * @details 親のスライド基準速度を 120（ダッシュ130の少し下）へ落としたのに合わせて
			 *          係数を上げ、滑り台感を維持している。約3度の下りでダッシュを追い越し、
			 *          16.8度（Normalの中央値）で 224 ≒ 泳ぎ、30度で 300。
			 *          「平地は走り、下りが見えたらスライド」の読み合いを作るのが狙い
			 *          （2026-08-23 試遊フィードバック）。
			 */
			constexpr float SLIDE_SLOPE_GAIN_DOWN = 3.0f;
			/**
			 * @brief 上りの罰
			 * @details 同フィードバック「上り坂は一気に減速し、滑り落ちていくぐらい」を受けて
			 *          0.85 から強化した。sin(θ)=0.286（約16.6度）の上りで速度が0になり、
			 *          それより急な上りでは倍率が負になって斜面をずり落ちる（親のみ。
			 *          m_isSlideBackAllowed 参照）。
			 */
			constexpr float SLIDE_SLOPE_GAIN_UP = 3.5f;
			/**
			 * @brief 速度倍率の上限
			 * @details 親の基準120に対して上限312。32度を超える下りで頭打ちになる。
			 *          ハイトマップ由来の法線の尖りで一瞬だけ極端な値が出る事故の対策も兼ねる。
			 */
			constexpr float SLIDE_SLOPE_MUL_MAX = 2.6f;
			/** 速度倍率の下限（AIの子ペンギン用。前進を保証して上り坂で永久後退しない） */
			constexpr float SLIDE_SLOPE_MUL_MIN = 0.25f;
			/** 速度倍率の下限（親ペンギン用。負＝斜面をずり落ちる。-1で基準速度と同速の後退まで） */
			constexpr float SLIDE_SLOPE_MUL_MIN_PLAYER = -1.0f;
			/**
			 * @brief 傾斜倍率の追従時間（秒）
			 * @details 尾根をまたいだ瞬間に目標速度が下り値と上り値を往復するのを防ぐ。
			 */
			constexpr float SLIDE_SLOPE_SMOOTH_TIME = 0.15f;
			/**
			 * @brief スライド中の旋回倍率の下限
			 * @details 速度倍率の上限 1.40 でも 1/1.40 = 0.71 にしかならないため、
			 *          現状は到達しない。「もっと氷らしく」と要望が出たときに
			 *          旋回倍率の式を強めるための余地として置いてある。
			 */
			constexpr float SLIDE_TURN_MUL_MIN = 0.60f;
			/** スタミナ消費倍率の上限（上りで無限に増えないようにする） */
			constexpr float SLIDE_STAMINA_DRAIN_SCALE_MAX = 2.0f;
		}

		void PenguinStateMachine::Jump()
		{
			m_ownerCharacter->GetCharacterController()->Jump(m_jumpPower);
			m_isJump = false; // ジャンプフラグをリセット
		}


		void PenguinStateMachine::Damage()
		{
			// デフォルト実装：各派生クラスでオーバーライド可能
		}


		PenguinStateMachine::PenguinStateMachine(PenguinBase* ownerPenguinBase)
			: CharacterStateMachine(ownerPenguinBase)
			, m_ownerPenguinBase(ownerPenguinBase)
			, m_jumpPower(0.0f)
			, m_isJump(false)
			, m_isSneak(false)
			, m_isSlide(false)
			, m_isDamaged(false)
			, m_isInWhirlpool(false)
			, m_jumpStaminaGauge(JUMP_STAMINA_MAX, JUMP_STAMINA_DECREASE_SPEED, JUMP_STAMINA_RECOVER_SPEED)
			, m_slideStaminaGauge(SLIDE_STAMINA_MAX, SLIDE_STAMINA_DECREASE_SPEED, SLIDE_STAMINA_RECOVER_SPEED)
		{}


		void PenguinStateMachine::UpdateStaminaGauges()
		{
			// まだ初期化できていなければここで試みる（Statusの値が揃うまで毎フレーム再試行される）
			if (!m_isStaminaGaugeSetup)
			{
				SetupStaminaGauges();
			}

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// 傾斜倍率の更新（スライド速度の算出に毎フレーム必要）
			UpdateSlideSlope();

			// スライドのスタミナは撤廃した（2026-08-23 試遊フィードバック）。
			// ゲージは消費させず満タンのまま維持する（CanUseSlide も常にtrue）
			m_slideStaminaGauge.Update(false, deltaTime);
			m_jumpStaminaGauge.Update(false, deltaTime);
		}


		void PenguinStateMachine::UpdateSlideSlope()
		{
			// 目標値の算出。接地情報が無い（ジャンプ中・上昇中・泳ぎ中）か
			// 移動入力が無いときは平地扱いに戻す
			float targetSigned = 0.0f;

			auto* controller = m_ownerCharacter->GetCharacterController();
			if (controller != nullptr
				&& controller->IsGroundInfoValid()
				&& m_moveDirection.LengthSq() > FLT_EPSILON)
			{
				// 移動入力を水平方向に正規化する。
				// m_moveDirection は親ペンギンではスティックの倒し量ぶんだけ短いことがあるため、
				// ここで正規化しないと傾斜が弱く見積もられてしまう
				Vector3 moveDir = m_moveDirection;
				moveDir.y = 0.0f;
				if (moveDir.LengthSq() > FLT_EPSILON)
				{
					moveDir.Normalize();

					// 地面法線は単位ベクトルなので |normal.xz| がそのまま sinθ になり、
					// 最急降下の水平方向も normal.xz と一致する。
					// よって「進行方向と最急降下方向の内積」= sinθ * cos(進行方向と下り方向の角度)
					// が2D内積1回で求まる（三角関数も平方根も不要）
					const Vector3& normal = controller->GetGroundNormal();
					targetSigned = moveDir.x * normal.x + moveDir.z * normal.z;

					// 法線が単位ベクトルでない事故に備えて範囲を保証する
					targetSigned = max(-1.0f, min(1.0f, targetSigned));
				}
			}

			// 法線のちらつきで速度が脈打たないよう時定数で追従させる
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const float lerpFactor = min(1.0f, deltaTime / SLIDE_SLOPE_SMOOTH_TIME);
			m_slideSlopeSigned += (targetSigned - m_slideSlopeSigned) * lerpFactor;

			// 符号つき傾斜を速度倍率へ変換する（下りと上りで係数が違う）
			const float gain = (m_slideSlopeSigned >= 0.0f) ? SLIDE_SLOPE_GAIN_DOWN : SLIDE_SLOPE_GAIN_UP;
			const float multiplier = 1.0f + gain * m_slideSlopeSigned;

			// 親ペンギンは急な上りで倍率が負になり、斜面をずり落ちる。
			// AIの子ペンギンは前進の下限を保証する（上り坂で永久に後退しないため）
			const float mulMin = m_isSlideBackAllowed ? SLIDE_SLOPE_MUL_MIN_PLAYER : SLIDE_SLOPE_MUL_MIN;
			m_slideSlopeMultiplier = max(mulMin, min(SLIDE_SLOPE_MUL_MAX, multiplier));
		}


		float PenguinStateMachine::CalcSlideSpeedWithSlope() const
		{
			const PenguinStatus* status = GetPenguinStatus();
			if (status == nullptr) return 0.0f;

			return status->GetSlideSpeed() * m_slideSlopeMultiplier;
		}


		float PenguinStateMachine::CalcSlideTurnMultiplier() const
		{
			// 上り・平地では操作性を落とさない（上りは既に遅いので二重の罰になる）
			if (m_slideSlopeMultiplier <= 1.0f) return 1.0f;

			// 加速しているぶんだけ曲がりにくくする
			return max(SLIDE_TURN_MUL_MIN, 1.0f / m_slideSlopeMultiplier);
		}


		void PenguinStateMachine::SetupStaminaGauges()
		{
			// すでに初期化済みなら何もしない（Statusのホットリロードで何度呼ばれても安全にする）
			if (m_isStaminaGaugeSetup) return;

			const PenguinStatus* status = GetPenguinStatus();
			if (!status) return;

			// Statusのセットアップがまだ済んでいない場合、最大値が0のままなので次のフレームに再試行する。
			if (status->GetJumpStaminaMax() <= 0.0f && status->GetSlideStaminaMax() <= 0.0f) return;

			m_jumpStaminaGauge.Initialize(status->GetJumpStaminaMax(), JUMP_STAMINA_DECREASE_SPEED, status->GetJumpStaminaRecoverSpeed());
			m_slideStaminaGauge.Initialize(status->GetSlideStaminaMax(), status->GetSlideStaminaDecreaseSpeed(), status->GetSlideStaminaRecoverSpeed());

			m_isStaminaGaugeSetup = true;
		}


		PenguinEffectStatus* PenguinStateMachine::GetEffectStatus() const
		{
			if (m_ownerPenguinBase)
			{
				return m_ownerPenguinBase->GetEffectStatus();
			}
			return nullptr;
		}


		core::IState* PenguinStateMachine::GetChangeState()
		{
			return nullptr;
		}


		const char* PenguinStateMachine::GetStateNameForLog() const
		{
			if (IsEqualCurrentState(PenguinDeadState::ID()))         return "Dead";
			if (IsEqualCurrentState(PenguinDiyingState::ID()))       return "Dying";
			if (IsEqualCurrentState(PenguinInWhirlpoolState::ID()))  return "InWhirlpool";
			if (IsEqualCurrentState(PenguinSwimmingState::ID()))     return "Swim";
			if (IsEqualCurrentState(PenguinDamagedState::ID()))      return "Damaged";
			if (IsEqualCurrentState(PenguinRunState::ID()))          return "Run";
			if (IsEqualCurrentState(PenguinSneakState::ID()))        return "Sneak";
			if (IsEqualCurrentState(PenguinJumpState::ID()))         return "Jump";
			if (IsEqualCurrentState(PenguinSlidingState::ID()))      return "Slide";
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))   return "SlideStart";
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))     return "SlideEnd";
			return "Idle";
		}
	}
}