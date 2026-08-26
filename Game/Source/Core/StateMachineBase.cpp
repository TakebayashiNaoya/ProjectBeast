/**
 * @file ActorStateMachine.cpp
 * @brief アクターのステートマシンの基底クラス群
 */
#include "stdafx.h"
#include "StateMachineBase.h"


namespace app
{
	namespace core
	{
		StateMachineBase::StateMachineBase()
			: m_currentState(nullptr)
			, m_nextState(nullptr)
		{
			m_stateMap.clear();
		}


		void StateMachineBase::Update()
		{
			// ステートを変更する
			ChangeState();

			// 現在のステートを更新する
			if (m_currentState) m_currentState->Update();
		}


		void StateMachineBase::ChangeState()
		{
			// 変更先のステートを取得する
			m_nextState = GetChangeState();

			// ステートが変更されている場合
			if (m_nextState && m_currentState != m_nextState)
			{
				if (m_currentState)
				{
					m_currentState->Exit();
				}
				m_currentState = m_nextState;
				m_currentState->Enter();
				m_nextState = nullptr;
			}
		}


		void StateMachineBase::ReEnterCurrentState()
		{
			// 現在のステートの Enter() を再度呼び出してアニメーションを再適用する
			if (m_currentState)
			{
				m_currentState->Enter();
			}
		}


		bool StateMachineBase::IsEqualCurrentState(const uint32_t stateID) const
		{
			return m_currentState == m_stateMap.at(stateID).get();
		}


		IState* StateMachineBase::FindState(const uint32_t stateID)
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