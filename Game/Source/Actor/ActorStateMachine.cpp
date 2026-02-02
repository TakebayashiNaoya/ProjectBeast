#include "stdafx.h"
#include "ActorStateMachine.h"




namespace app
{
	namespace actor
	{
		IState::IState()
			: m_stateMachine(nullptr)
		{
		}



		/***************************************/


		void ActorStateMachine::Update()
		{
			// ステートを変更する
			ChangeState();

			// 現在のステートを更新する
			if (m_currentState) m_currentState->Update();
		}


		void ActorStateMachine::ChangeState()
		{
			m_nextState = GetChangeState();

			if (m_nextState && m_currentState == m_nextState)
			{
				m_currentState->Exit();
				m_currentState = m_nextState;
				m_currentState->Enter();
				m_nextState = nullptr;
			}
		}


		IState* ActorStateMachine::FindState(const uint8_t stateID)
		{
			const auto& it = m_stateMap.find(stateID);

			if (it == m_stateMap.end()) return nullptr;

			return it->second.get();
		}


		ActorStateMachine::ActorStateMachine()
			: m_currentState(nullptr)
			, m_nextState(nullptr)
		{
			m_stateMap.clear();
		}
	}
}