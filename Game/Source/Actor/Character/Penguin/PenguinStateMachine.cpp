/**
 * @file PenguinStateMachine.cpp
 * @brief ペンギンのステートマシン
 */
#include "stdafx.h"
#include "PenguinBase.h"
#include "PenguinIState.h"
#include "PenguinSlideRule.h"
#include "PenguinStateMachine.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			//============================================//
			// スライドの傾斜モデル
			// 各定数の根拠と数値表は docs/スライドの傾斜モデル.md にある。
			// 触るときは必ずそちらの表も更新すること。
			//============================================//

			/**
			 * @brief 下りの効き
			 * @details 親の基準速度200で、3.4度の下りで236、16.8度（Normalの中央値）で373、30度で500。
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
			 * @details 親の基準200に対して上限520。32度を超える下りで頭打ちになる。
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
		}


		void PenguinStateMachine::Damage()
		{
			// デフォルト実装：各派生クラスでオーバーライド可能
		}


		PenguinStateMachine::PenguinStateMachine(PenguinBase* ownerPenguinBase)
			: CharacterStateMachine(ownerPenguinBase)
			, m_ownerPenguinBase(ownerPenguinBase)
			, m_isSneak(false)
			, m_isSlide(false)
			, m_isDamaged(false)
			, m_isInWhirlpool(false)
		{}


		void PenguinStateMachine::UpdateSlideSlope()
		{
			// 目標値の算出。接地情報が無い（落下中・泳ぎ中）か
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


		PenguinStateMachine::SlideSpeeds PenguinStateMachine::CalcSlideSpeeds(const Vector3& moveDirection, const bool isDash) const
		{
			SlideSpeeds speeds = { 0.0f, 0.0f, 0.0f };

			// 現在の水平速度
			Vector3 horizontalVelocity = m_currentVelocity;
			horizontalVelocity.y = 0.0f;
			speeds.current = horizontalVelocity.Length();

			const PenguinStatus* status = GetPenguinStatus();
			if (status == nullptr) return speeds;

			// Move()と同じく倒し量と陣形倍率を掛ける（目標手前のブレーキは含めない）
			const float multiplier = moveDirection.Length() * m_externalSpeedMultiplier;
			speeds.slope = CalcSlideSpeedWithSlope() * multiplier;
			speeds.standing = (isDash ? status->GetRunSpeed() : status->GetSneakSpeed()) * multiplier;
			return speeds;
		}


		float PenguinStateMachine::CalcSlideMoveSpeed() const
		{
			// 勢いだけで滑っているときは通常の移動速度まで落としていく
			if (m_isSlideCoast)
			{
				const PenguinStatus* status = GetPenguinStatus();
				if (status == nullptr) return 0.0f;
				return m_isDash ? status->GetRunSpeed() : status->GetSneakSpeed();
			}
			return CalcSlideSpeedWithSlope();
		}


		float PenguinStateMachine::CalcSlideCoastDeceleration() const
		{
			// 急な上りではすぐ減速する
			if (!slideRule::UsesCoast(m_slideSlopeSigned)) return 0.0f;

			// 陣形の速度倍率が変わっても滑り続ける時間を揃える
			return m_slideCoastDeceleration * m_externalSpeedMultiplier;
		}


		float PenguinStateMachine::CalcSlideTurnMultiplier() const
		{
			// 上り・平地では操作性を落とさない（上りは既に遅いので二重の罰になる）
			if (m_slideSlopeMultiplier <= 1.0f) return 1.0f;

			// 加速しているぶんだけ曲がりにくくする
			return max(SLIDE_TURN_MUL_MIN, 1.0f / m_slideSlopeMultiplier);
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
			if (IsEqualCurrentState(PenguinFallState::ID()))         return "Jump"; // 過去のリプレイログと揃える
			if (IsEqualCurrentState(PenguinSlidingState::ID()))      return "Slide";
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))   return "SlideStart";
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))     return "SlideEnd";
			return "Idle";
		}
	}
}