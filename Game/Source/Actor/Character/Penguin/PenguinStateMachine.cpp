/**
 * @file PenguinStateMachine.cpp
 * @brief ペンギンのステートマシン
 * @author 藤谷
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
			 * @details 新 Normal に実在する最大傾斜 41.4 度で、親のスライド 180 が
			 *          泳ぎ 230 に並ぶよう決めた（230/180 = 1.278、0.278/sin(41.4) = 0.42）。
			 *          泳ぎが最速の移動手段である立場を崩さないための上限でもある。
			 */
			constexpr float SLIDE_SLOPE_GAIN_DOWN = 0.42f;
			/**
			 * @brief 上りの罰
			 * @details 下りの倍以上にしてある。傾斜 16.8 度（新 Normal の中央値）の上りで
			 *          親のスライドが 136 となり走り 130 とほぼ同値、31.7 度で 100 と
			 *          走りより遅くなる。「スライドを押しっぱなしにするのが常に最適」に
			 *          ならない角度を 20 度前後へ置くための値。
			 */
			constexpr float SLIDE_SLOPE_GAIN_UP = 0.85f;
			/**
			 * @brief 速度倍率の上限
			 * @details 接地限界 63 度の値（x1.374）のすぐ上に置いてあるので、
			 *          正常な地形ではクランプされない。ハイトマップ由来の法線が
			 *          1ピクセルの尖りで極端な値を返したときの事故対策。
			 */
			constexpr float SLIDE_SLOPE_MUL_MAX = 1.40f;
			/** 速度倍率の下限（速度が0や負にならないことの保証） */
			constexpr float SLIDE_SLOPE_MUL_MIN = 0.25f;
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

			// スタミナ消費に傾斜倍率が要るので、ゲージより先に傾斜を更新する
			UpdateSlideSlope();

			// 下りほど消費が減り、上りほど増える。真下り（s=1）で消費0になる連続関数のため、
			// 「消費する／しない」の境界で挙動が飛ばない
			float drainScale = 1.0f - m_slideSlopeSigned;
			drainScale = max(0.0f, min(SLIDE_STAMINA_DRAIN_SCALE_MAX, drainScale));

			m_slideStaminaGauge.Update(m_isSlide, deltaTime, drainScale);
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
			m_slideSlopeMultiplier = max(SLIDE_SLOPE_MUL_MIN, min(SLIDE_SLOPE_MUL_MAX, multiplier));
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