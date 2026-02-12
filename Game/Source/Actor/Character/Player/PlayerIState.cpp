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

		PlayerIState::PlayerIState(PlayerStateMachine* owner)
			: m_owner(owner)
		{
		}




		/************************************/


		void PlayerIdleState::Enter()
		{
		}


		void PlayerIdleState::Update()
		{
		}


		void PlayerIdleState::Exit()
		{
		}


		PlayerIdleState::PlayerIdleState(PlayerStateMachine* owner)
			: PlayerIState(owner)
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


		PlayerMoveState::PlayerMoveState(PlayerStateMachine* owner)
			: PlayerIState(owner)
		{
		}
	}
}