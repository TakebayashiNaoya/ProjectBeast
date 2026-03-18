/**
 * @file EnemyStateMachine.cpp
 * @brief エネミーのステートマシン
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyIState.h"
#include "EnemyStateMachine.h"

#include "EnemyStatus.h"


namespace app
{
	namespace actor
	{
		EnemyStateMachine::EnemyStateMachine(Enemy* enemy)
			: CharacterStateMachine(enemy)
			, m_owner(enemy)
		{
			// ステートの追加
			AddState<EnemyIdleState>(this);
			AddState<EnemyWanderingState>(this);
			AddState<EnemyChaseState>(this);
			AddState<EnemyJumpState>(this);
			AddState<EnemySwimState>(this);
			AddState<EnemyAttackState>(this);


			// 初期ステートの設定
			m_currentState = FindState(EnemyWanderingState::ID());

			m_transform.m_position = Vector3(80.0f, 0.0f, 800.0f);
		}


		void EnemyStateMachine::Update()
		{
			//ステートの切り替え
			ChangeState();
			//ステートの更新処理
			m_currentState->Update();
		}


		void EnemyStateMachine::ChangeState()
		{
			// ステートの切り替え
			m_nextState = GetChangeState();
			//ステートが切り替わった時・今のステートがnextStateと同じ数字ではないとき
			if (m_nextState != nullptr && m_currentState != m_nextState) {
				//今のステートを終了
				m_currentState->Exit();
				//次のステートに切り替える
				m_currentState = m_nextState;
				//次のステートに入る
				m_currentState->Enter();
				//次のステートを初期化
				m_nextState = nullptr;
			}
		}


		core::IState* EnemyStateMachine::GetChangeState()
		{
			if (CanChangeIdle())
			{
				return FindState(EnemyIdleState::ID());
			}
			if (CanChangeAttack())
			{
				return FindState(EnemyAttackState::ID());
			}
			if (CanChangeChace())
			{
				return FindState(EnemyChaseState::ID());
			}
			if (CanChangeWandering())
			{
				return FindState(EnemyWanderingState::ID());
			}

			return FindState(EnemyIdleState::ID());
		}


		bool EnemyStateMachine::CanChangeIdle() const
		{
			if (m_stickLAmount < 0.01f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeWandering() const
		{
			if (!m_isFindPenguin && m_stickLAmount > 0.01f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeChace() const
		{
			if (m_isFindPenguin && m_stickLAmount > 0.01f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeAttack() const
		{
			//if (!m_canAttack)return false;
			if (!m_isNearPenguin)return false;

			if (m_actionButtonX) {
				return true;
			}
			return false;
		}


		void EnemyStateMachine::Setup(Enemy* owner)
		{
			m_owner = owner;

		}

		const EnemyStatus* EnemyStateMachine::GetOwnerStatus()
		{
			return m_owner->GetStatus<EnemyStatus>();
		}
	}
}