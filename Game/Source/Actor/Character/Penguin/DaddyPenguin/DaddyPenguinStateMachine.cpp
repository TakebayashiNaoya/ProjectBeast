/**
 * @file DaddyPenguinStateMachine.cpp
 * @brief 親ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* player)
			: StateMachineBase()
			, m_player(player)
		{
			// ステートの追加
			AddState<DaddyPenguinIdleState>(this);
			AddState<DaddyPenguinMoveState>(this);

			// 初期ステートの設定
			m_currentState = FindState(DaddyPenguinIdleState::ID());
		}


		void DaddyPenguinStateMachine::PlayAnimation(const uint8_t animationID)
		{
			m_player->GetModelRender()->PlayAnimation(animationID);
		}


		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			return FindState(DaddyPenguinIdleState::ID());
			return nullptr;
		}
	}
}