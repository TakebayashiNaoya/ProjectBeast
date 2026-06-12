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
			, m_isSearch(false)
			, m_isFindPenguin(false)
			, m_isNearPenguin(false)
			, m_canAttack(false)
			, m_isStun(false)
			, m_isReturnHome(false)
			, m_isCoolDown(false)
			, m_isAttackPlaying(false)
			, m_isRoar(false)
			, m_isChasing(false)
			, m_isAttackImpact(false)
			, m_wakeUpGauge(0.0f)
			, m_sleepTimer(0.0f)
			, m_searchTargetPos(Vector3::Zero)
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
			AddState<EnemyRoarState>(this);

			// 初期ステートの設定
			m_currentState = FindState(EnemyIdleState::ID());

			// TODO: 初期座標のハードコーディング。必要に応じてパラメータ化を検討
			m_transform.m_position = Vector3(0.0f, 10.0f, 100.0f);
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
			// ① クールダウン中は絶対維持
			if (m_currentState == FindState(EnemyCoolDownState::ID()))
			{
				if (m_isCoolDown) return FindState(EnemyCoolDownState::ID());
			}

			// ② スタンは最優先
			if (CanChangeStun()) return FindState(EnemyStunState::ID());

			// ③ 攻撃も水中より優先（水中でも攻撃できるようにする）
			if (CanChangeAttack()) return FindState(EnemyAttackState::ID());

			// ④ 咆哮
			if (CanChangeRoar()) return FindState(EnemyRoarState::ID());

			// ⑤ 泳ぎ判定（攻撃・スタンより後）
			if (m_currentState == FindState(EnemySwimState::ID()))
			{
				if (IsInWater() || !IsOnGround()) return FindState(EnemySwimState::ID());
				// 陸に上がったら以下の判定へ続く
			}
			else if (CanChangeSwimState())
			{
				return FindState(EnemySwimState::ID());
			}

			// ⑥ 以降は既存の優先度
			if (CanChangeReturnHome()) return FindState(EnemyReturnHomeState::ID());
			if (CanChangeChase())      return FindState(EnemyChaseState::ID());
			if (CanChangeSearch())     return FindState(EnemySearchState::ID());
			if (CanChangeWalk())       return FindState(EnemyWalkState::ID());
			if (CanChangeCoolDown())   return FindState(EnemyCoolDownState::ID());

			return FindState(EnemyIdleState::ID());
		}


		bool EnemyStateMachine::CanChangeIdle() const
		{
			if (m_stickLAmount < STICK_INPUT_THRESHOLD) {
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
			if (m_isSearch && m_stickLAmount >= STICK_INPUT_THRESHOLD)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeWalk() const
		{
			if (m_stickLAmount >= STICK_INPUT_THRESHOLD) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeReturnHome() const
		{
			if (m_isReturnHome)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeChase() const
		{
			if (m_actionButtonB && m_stickLAmount > STICK_INPUT_THRESHOLD) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeAttack() const
		{
			if (!m_isNearPenguin) return false;
			if (m_actionButtonX) {
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeCoolDown() const
		{
			if (m_isCoolDown)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::CanChangeRoar() const
		{
			if (m_isRoar)
			{
				return true;
			}
			return false;
		}


		bool EnemyStateMachine::IsSwim() const
		{
			return IsInWater();
		}


		void EnemyStateMachine::Setup(Enemy* owner)
		{
			m_owner = owner;
		}


		const EnemyStatus* EnemyStateMachine::GetOwnerStatus()
		{
			return m_owner->GetStatus<EnemyStatus>();
		}


		EnemyStatus* EnemyStateMachine::GetOwnerStatusMutable()
		{
			return m_owner->GetStatus<EnemyStatus>();
		}


		const char* EnemyStateMachine::GetStateNameForLog() const
		{
			if (IsEqualCurrentState(EnemyCoolDownState::ID())) return "Sleep";
			if (IsEqualCurrentState(EnemyChaseState::ID()))    return "Chase";
			if (IsEqualCurrentState(EnemyAttackState::ID()))   return "Attack";
			if (IsEqualCurrentState(EnemyRoarState::ID()))     return "Roar";
			if (IsEqualCurrentState(EnemyStunState::ID()))     return "Stun";
			if (IsEqualCurrentState(EnemySearchState::ID()))   return "Search";
			if (IsEqualCurrentState(EnemyWalkState::ID()))     return "Walk";
			if (IsEqualCurrentState(EnemySwimState::ID()))     return "Swim";
			if (IsEqualCurrentState(EnemyJumpState::ID()))     return "Jump";
			if (IsEqualCurrentState(EnemyReturnHomeState::ID())) return "Return";
			return "Idle";
		}
	}
}