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
			: StateMachineBase()
			, m_player(player)
		{
			// ステートの追加
			AddState<PlayerIdleState>(this);
			AddState<PlayerMoveState>(this);

			// 初期ステートの設定
			m_currentState = FindState(PlayerIdleState::ID());
		}


		IState* PlayerStateMachine::GetChangeState()
		{
			return FindState(PlayerIdleState::ID());
			return nullptr;
		}
	}
}