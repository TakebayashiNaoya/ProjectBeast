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
			, m_ownerStatus(nullptr)
			, m_currentState(nullptr)
			, m_nextState(nullptr)
			, m_moveVector(Vector3::Zero)
			, m_playerPosition(Vector3::Zero)
			, m_targetPlayer(nullptr)
			, m_stickLAmount(0.0f)
			, m_actionButtonA(false)
			, m_actionButtonB(false)
			, m_actionButtonX(false)
			, m_isFindPenguin(false)
			, m_isNearPenguin(false)
			, m_canAttack(false)
			, m_isSeach(false)
			, m_isReturnHome(false)
		{
			// ステートの追加
			AddState<EnemyIdleState>(this);
			AddState<EnemyStunState>(this);
			AddState<EnemySearchState>(this);
			AddState<EnemyWalkState>(this);
			AddState<EnemyChaseState>(this);
			AddState<EnemyJumpState>(this);
			AddState<EnemySwimState>(this);
			AddState<EnemyAttackState>(this);
			AddState<EnemyReturnHomeState>(this);
			AddState<EnemyCoolDownState>(this);


			// 初期ステートの設定
			m_currentState = FindState(EnemyIdleState::ID());

			m_transform.m_position = Vector3(0.0f, -80.0f, 100.0f);
			m_transform.m_position;

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
			if (CanChangeReturnHome())
			{
				return FindState(EnemyReturnHomeState::ID());
			}
			if (CanChangeStun())
			{
				return FindState(EnemyStunState::ID());
			}

			if (CanChangeAttack())
			{
				return FindState(EnemyAttackState::ID());
			}
			if (CanChangeChace())
			{
				return FindState(EnemyChaseState::ID());
			}
			if (CanChangeSearch())
			{
				return FindState(EnemySearchState::ID());
			}
			if (CanChangeSwimState())
			{
				return FindState(EnemySwimState::ID());
			}
			if (CanChangeWalk())
			{
				return FindState(EnemyWalkState::ID());
			}
			if (CanChangeCoolDown())
			{
				return FindState(EnemyCoolDownState::ID());
			}

			return FindState(EnemyIdleState::ID());
		}


		bool EnemyStateMachine::CanChangeIdle() const
		{
			if (m_stickLAmount < 0.0001f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeStun() const
		{
			if (m_isStun)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeSearch() const
		{
			if (m_isSeach && m_stickLAmount >= 0.0001f)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeWalk() const
		{
			if (m_stickLAmount >= 0.0001f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeReturnHome()const
		{
			if (m_isReturnHome)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeChace() const
		{
			if (m_actionButtonB && m_stickLAmount > 0.0001f) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeAttack() const
		{
			if (!m_isNearPenguin)return false;
			if (m_actionButtonX) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeCoolDown()const
		{
			if (m_isCoolDown)
			{
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