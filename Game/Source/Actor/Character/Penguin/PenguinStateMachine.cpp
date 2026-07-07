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

			m_slideStaminaGauge.Update(m_isSlide, deltaTime);
			m_jumpStaminaGauge.Update(false, deltaTime);
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