/**
 * @file ActorStateMachine.cpp
 * @brief アクターのステートマシンの基底クラス群
 * @author 藤谷
 */
#include "stdafx.h"
#include "ActorStateMachine.h"


namespace app
{
	namespace actor
	{
		IState::IState(ActorStateMachine* stateMachine)
			: m_stateMachine(stateMachine)
		{
		}




		/***************************************/


		ActorStateMachine::ActorStateMachine()
			: m_currentState(nullptr)
			, m_nextState(nullptr)
		{
			m_stateMap.clear();
		}


		void ActorStateMachine::Update()
		{
			// ステートを変更する
			ChangeState();

			// 現在のステートを更新する
			if (m_currentState) m_currentState->Update();
		}


		void ActorStateMachine::ChangeState()
		{
			// 変更先のステートを取得する
			m_nextState = GetChangeState();

			// ステートが変更されている場合
			if (m_nextState && m_currentState != m_nextState)
			{
				m_currentState->Exit();
				m_currentState = m_nextState;
				m_currentState->Enter();
				m_nextState = nullptr;
			}
		}


		IState* ActorStateMachine::FindState(const uint8_t stateID)
		{
			// 指定したIDを取得
			const auto& it = m_stateMap.find(stateID);
			// IDが外れ値の場合nullptrを返す
			if (it == m_stateMap.end()) return nullptr;
			// ステートを返す
			return it->second.get();
		}
	}
}