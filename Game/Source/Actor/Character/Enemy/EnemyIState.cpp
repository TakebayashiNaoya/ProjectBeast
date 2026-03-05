/**
 * @file EnemyIState.cpp
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyIState.h"


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
		}


		void EnemyWanderingState::Exit()
		{
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

		}


		void EnemyChaceState::Exit()
		{

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