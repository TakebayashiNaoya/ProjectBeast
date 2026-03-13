/**
 * @file ChildPenguinStateMachine.cpp
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinIState.h"
#include "ChildPenguinStateMachine.h"


namespace app
{
	namespace actor
	{
		ChildPenguinStateMachine::ChildPenguinStateMachine(ChildPenguin* player)
			: StateMachineBase()
			, m_player(player)
		{
			// ステートの追加
			AddState<ChildPenguinIdleState>(this);
			AddState<ChildPenguinMoveState>(this);

			// 初期ステートの設定
			m_currentState = FindState(ChildPenguinIdleState::ID());
		}


		void ChildPenguinStateMachine::PlayAnimation(const uint8_t animationID)
		{
			m_player->GetModelRender()->PlayAnimation(animationID);
		}


		core::IState* ChildPenguinStateMachine::GetChangeState()
		{
			return FindState(ChildPenguinIdleState::ID());
			return nullptr;
		}
	}
}