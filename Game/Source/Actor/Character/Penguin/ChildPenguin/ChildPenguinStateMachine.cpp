/**
 * @file ChildPenguinStateMachine.cpp
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinIState.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"


namespace app
{
	namespace actor
	{
		ChildPenguinStateMachine::ChildPenguinStateMachine(ChildPenguin* ownerChildPenguin)
			: CharacterStateMachine(ownerChildPenguin)
			, m_ownerChildPenguin(ownerChildPenguin)
		{
			// ステートの追加
			AddState<ChildPenguinIdleState>(this);
			AddState<ChildPenguinMoveState>(this);

			// 初期ステートの設定
			m_currentState = FindState(ChildPenguinIdleState::ID());
		}


		const ChildPenguinStatus* ChildPenguinStateMachine::GetChildPenuinStatus() const
		{
			return m_ownerChildPenguin->GetStatus<ChildPenguinStatus>();
		}


		core::IState* ChildPenguinStateMachine::GetChangeState()
		{
			return FindState(ChildPenguinIdleState::ID());
			return nullptr;
		}
	}
}