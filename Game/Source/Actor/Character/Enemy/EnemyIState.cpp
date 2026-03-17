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
		}


		void EnemyIdleState::Update()
		{}


		void EnemyIdleState::Exit()
		{}


		EnemyIdleState::EnemyIdleState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyWanderingState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Walk);
		}


		void EnemyWanderingState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.01f) {
				return;
			}
			m_owner->Move();
		}


		void EnemyWanderingState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}


		EnemyWanderingState::EnemyWanderingState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyChaseState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
		}


		void EnemyChaseState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.01f) {
				return;
			}

			m_owner->Move();
		}


		void EnemyChaseState::Exit()
		{
			//m_owner->SetMoveVector(Vector3::Zero);
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

		}


		void EnemySwimState::Update()
		{

		}


		void EnemySwimState::Exit()
		{

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
		{}


		void EnemyAttackState::Exit()
		{

		}


		EnemyAttackState::EnemyAttackState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}
	}
}