/**
 * @file EnemyIState.cpp
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyController.h"
#include "EnemyIState.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"
#include "EnemyTypes.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{
		EnemyIState::EnemyIState(EnemyStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void EnemyIdleState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Idle);


			SoundManager::Get().PlaySE(enSoundKind_EnemyGrowl);

		}


		void EnemyIdleState::Update()
		{}


		void EnemyIdleState::Exit()
		{}


		EnemyIdleState::EnemyIdleState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/




		void EnemyStunState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Stun);
			m_owner->SetStickLAmount(0.0f); // 動かない

			m_stunTimer = 2.0f;
		}


		void EnemyStunState::Update()
		{
			m_stunTimer -= g_gameTime->GetFrameDeltaTime();

			if (!m_owner->IsPlayingAnimation() || m_stunTimer <= 0.0f)
			{
				m_owner->SetStun(false); // ←ここで解除！
			}
		}

		void EnemyStunState::Exit()
		{
			m_owner->SetStun(false);
		}

		EnemyStunState::EnemyStunState(EnemyStateMachine* owner)
			:EnemyIState(owner),
			m_stunTimer(0.0f)
		{

		}


		/************************************/



		void EnemySearchState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::BackWalk);
			if (rand() % 100 < 30)
			{
				SoundManager::Get().PlaySE(enSoundKind_EnemyGrowl);
			}
			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemySearchState::Update()
		{
			m_owner->Move();
		}


		void EnemySearchState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemySearchState::EnemySearchState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyWalkState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Walk);

			if (m_stepSE == -1)
			{
				m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true, false);
			}
		}


		void EnemyWalkState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}
			m_owner->Move();

			if (m_stepSE == -1)
			{
				m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);
			}
		}


		void EnemyWalkState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyWalkState::EnemyWalkState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyChaseState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Run);


			SoundManager::Get().PlaySE(enSoundKind_EnemyRoar);

			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemyChaseState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}

			m_owner->Move();
		}


		void EnemyChaseState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyChaseState::EnemyChaseState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyJumpState::Enter()
		{

		}


		void EnemyJumpState::Update()
		{

		}


		void EnemyJumpState::Exit()
		{

		}


		EnemyJumpState::EnemyJumpState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{

		}




		/************************************/


		void EnemySwimState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Run);
		}


		void EnemySwimState::Update()
		{
			m_owner->Move();
		}


		void EnemySwimState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}

		EnemySwimState::EnemySwimState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{

		}




		/************************************/


		void EnemyAttackState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Attack);
		}


		void EnemyAttackState::Update()
		{

		}


		void EnemyAttackState::Exit()
		{}


		EnemyAttackState::EnemyAttackState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyReturnHomeState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Walk);

			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemyReturnHomeState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}
			m_owner->Move();
		}


		void EnemyReturnHomeState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyReturnHomeState::EnemyReturnHomeState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyCoolDownState::Enter()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			m_owner->PlayAnimation(EnEnemyAnimationType::Sleep);
		}


		void EnemyCoolDownState::Update()
		{}


		void EnemyCoolDownState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}


		EnemyCoolDownState::EnemyCoolDownState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}
	}
}