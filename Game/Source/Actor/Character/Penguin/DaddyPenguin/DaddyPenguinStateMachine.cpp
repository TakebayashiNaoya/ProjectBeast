/**
 * @file DaddyPenguinStateMachine.cpp
 * @brief 親ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin)
			: CharacterStateMachine(ownerDaddyPenguin)
			, m_ownerDaddyPenguin(ownerDaddyPenguin)
		{
			// ステートの追加
			AddState<DaddyPenguinIdleState>(this);
			AddState<DaddyPenguinSneakState>(this);

			// 初期ステートの設定
			m_currentState = FindState(DaddyPenguinSneakState::ID());
			m_currentState->Enter();
		}


		const DaddyPenguinStatus* DaddyPenguinStateMachine::GetDaddyPenguinStatus() const
		{
			return m_ownerDaddyPenguin->GetStatus<DaddyPenguinStatus>();
		}


		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			return FindState(DaddyPenguinSneakState::ID());
			return nullptr;
		}
	}
}