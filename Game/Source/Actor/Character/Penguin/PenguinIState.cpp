/**
 * @file PenguinIState.cpp
 * @brief ペンギン共通のステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinAnimationData.h"
#include "PenguinIState.h"
#include "PenguinStateMachine.h"
#include "PenguinStatus.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{

		PenguinIState::PenguinIState(PenguinStateMachine* owner)
			: m_owner(owner)
			, m_seHandle(-1)
		{}




		/************************************/


		void PenguinIdleState::Enter()
		{
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);
		}


		void PenguinIdleState::Update()
		{
			m_owner->Move();
		}


		void PenguinIdleState::Exit()
		{}


		PenguinIdleState::PenguinIdleState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSneakState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveWalk);
			soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSneak, true);
		}


		void PenguinSneakState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Sneak
			);
		}


		void PenguinSneakState::Exit()
		{
			SoundManager::Get().StopSE(soundHandle);
		}


		PenguinSneakState::PenguinSneakState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinRunState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveRun);
			soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinDash, true);
		}


		void PenguinRunState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Dash
			);
		}


		void PenguinRunState::Exit()
		{
			SoundManager::Get().StopSE(soundHandle);
		}


		PenguinRunState::PenguinRunState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinJumpState::Enter()
		{
			const float jumpPower = m_owner->GetPenguinStatus()->GetJumpPower();
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);

			// ボタン入力による正規のジャンプ遷移の場合のみJump()を呼ぶ（崖からの落下時は呼ばない）
			if (m_owner->GetIsJump())
			{
				m_owner->SetJumpPower(jumpPower);
				m_owner->Jump();
			}

			m_owner->PlayAnimation(EnPenguinAnimationID::JumpWalking);
			SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinJump, false);
		}


		void PenguinJumpState::Update()
		{
			m_owner->Move();
		}


		void PenguinJumpState::Exit()
		{
			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Fall
			);
		}


		PenguinJumpState::PenguinJumpState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideStartState::Enter()
		{
			// 滑るステートと同じ速度を設定
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::SlideStart);
		}


		void PenguinSlideStartState::Update()
		{
			// 移動を可能にする
			m_owner->Move();
		}


		void PenguinSlideStartState::Exit()
		{}


		PenguinSlideStartState::PenguinSlideStartState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlidingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::Sliding);
			soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSlide, true);
		}


		void PenguinSlidingState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Slide
			);
		}


		void PenguinSlidingState::Exit()
		{
			SoundManager::Get().StopSE(soundHandle);
		}


		PenguinSlidingState::PenguinSlidingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideEndState::Enter()
		{
			// 滑るステートと同じ速度を設定
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::StandUp);
		}


		void PenguinSlideEndState::Update()
		{
			// 移動を可能にする
			m_owner->Move();
		}


		void PenguinSlideEndState::Exit()
		{}


		PenguinSlideEndState::PenguinSlideEndState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinSwimmingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveSwim);
			if (m_seHandle == -1)
			{
				m_seHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinWaterIn, false);
			}
		}


		void PenguinSwimmingState::Update()
		{
			SoundManager* sound = &SoundManager::Get();
			if (m_seHandle != -1)
			{
				auto* se = sound->FindSE(m_seHandle);
				if (se && se->IsPlaying()) {
					sound->StopSE(m_seHandle);
					m_seHandle = sound->PlaySE(enSoundKind::enSoundKind_PenguinSwimming, true);
				}
			}
			m_owner->Move();
		}


		void PenguinSwimmingState::Exit()
		{
			SoundManager* sound = &SoundManager::Get();
			if (m_seHandle != -1)
			{
				auto* se = sound->FindSE(m_seHandle);
				if (se && se->IsPlaying()) {
					sound->StopSE(m_seHandle);
					m_seHandle = -1;
					sound->PlaySE(enSoundKind::enSoundKind_PenguinWaterOut, false);
				}
			}
		}


		PenguinSwimmingState::PenguinSwimmingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinDamagedState::Enter()
		{
			m_owner->Damage();
		}


		void PenguinDamagedState::Update()
		{}


		void PenguinDamagedState::Exit()
		{}


		PenguinDamagedState::PenguinDamagedState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDiyingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::DeathFaceDown);
		}


		void PenguinDiyingState::Update()
		{}


		void PenguinDiyingState::Exit()
		{}


		PenguinDiyingState::PenguinDiyingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDeadState::Enter()
		{
			// 死亡時の処理を派生ステートマシンに委譲する。
			// ChildPenguinStateMachineはここでマネージャーからの削除とdeleteを行う。
			m_owner->OnDead();
		}


		void PenguinDeadState::Update()
		{}


		void PenguinDeadState::Exit()
		{}


		PenguinDeadState::PenguinDeadState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}
	}
}