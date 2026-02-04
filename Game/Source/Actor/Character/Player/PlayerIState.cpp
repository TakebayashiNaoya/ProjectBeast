/**
 * @file PlayerIState.cpp
 * @brief プレイヤーのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "PlayerIState.h"
#include "PlayerStateMachine.h"
#include "PlayerStatus.h"


namespace app
{
	namespace actor
	{
		void PlayerIdleState::Enter()
		{
		}


		void PlayerIdleState::Update()
		{
		}


		void PlayerIdleState::Exit()
		{
		}

		PlayerIdleState::PlayerIdleState(ActorStateMachine* stateMachine)
			: IState(stateMachine)
			, m_owner(GetOwner<PlayerStateMachine>())
		{
		}




		/************************************/


		void PlayerMoveState::Enter()
		{
		}

		void PlayerMoveState::Update()
		{
		}

		void PlayerMoveState::Exit()
		{
		}

		PlayerMoveState::PlayerMoveState(ActorStateMachine* stateMachine)
			: IState(stateMachine)
			, m_owner(GetOwner<PlayerStateMachine>())
		{
		}
	}
}