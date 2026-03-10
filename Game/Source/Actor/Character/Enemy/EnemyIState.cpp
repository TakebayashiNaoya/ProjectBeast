/**
 * @file EnemyIState.cpp
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyIState.h"
#include "EnemyStateMachine.h"


namespace app
{
	namespace actor
	{
		EnemyIState::EnemyIState(EnemyStateMachine* owner)
			: m_owner(owner)
		{
		}




		/************************************/


		void EnemyIdleState::Enter()
		{
		}


		void EnemyIdleState::Update()
		{
		}


		void EnemyIdleState::Exit()
		{
		}


		EnemyIdleState::EnemyIdleState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{
		}




		/************************************/


		void EnemyWanderingState::Enter()
		{

		}


		void EnemyWanderingState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.01f) {
				return;
			}

			const Vector3& moveDirection = m_owner->GetDirection();
			// NOTE:Statusの設定が出来たらコメントを解除する
			//const Vector3 move = m_owner->GetOwnerStatus()->GetSpeed();
			//m_owner->SetMoveVector(move);
		}


		void EnemyWanderingState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}


		EnemyWanderingState::EnemyWanderingState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{
		}




		/************************************/


		void EnemyChaceState::Enter()
		{

		}


		void EnemyChaceState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.01f) {
				return;
			}

			const Vector3& moveDirection = m_owner->GetDirection();

			// NOTE:Statusの設定が出来たらコメントを解除する
			//const float moveSpeed = m_owner->GetOwnerStatus()->GetSpeed();
			//const float dashSpeed = m_owner->GetOwnerStatus()->GetDashSpeed();
			//const float moveDashSpeed = moveSpeed * dashSpeed;

			//const Vector3 move = moveDirection * moveDashSpeed;

			//m_owner->SetMoveVector(move);
		}


		void EnemyChaceState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}


		EnemyChaceState::EnemyChaceState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{
		}




		/************************************/


		void EnemyAttackState::Enter()
		{

		}


		void EnemyAttackState::Update()
		{
		}


		void EnemyAttackState::Exit()
		{

		}


		EnemyAttackState::EnemyAttackState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{
		}
	}
}