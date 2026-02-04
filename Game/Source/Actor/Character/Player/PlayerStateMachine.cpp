/**
 * @file PlayerStateMachine.cpp
 * @brief プレイヤーのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "PlayerIState.h"
#include "PlayerStateMachine.h"


namespace app
{
	namespace actor
	{
		PlayerStateMachine::PlayerStateMachine(Player* player)
			: ActorStateMachine()
			, m_player(player)
		{
		}


		IState* PlayerStateMachine::GetChangeState()
		{
			return nullptr;
		}
	}
}