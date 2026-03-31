/**
 * @file ChildPenguinStateMachine.cpp
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinManager.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"


namespace app
{
	namespace actor
	{
		ChildPenguinStateMachine::ChildPenguinStateMachine(ChildPenguin* ownerChildPenguin, EnChildPenguinType type)
			: PenguinStateMachine(ownerChildPenguin)
			, m_ownerChildPenguin(ownerChildPenguin)
			, m_type(type)
		{
			/** 共通ステートの追加 */
			AddState<PenguinIdleState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSneakState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinRunState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinJumpState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlideStartState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlidingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlideEndState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSwimmingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDamagedState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDiyingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDeadState>(static_cast<PenguinStateMachine*>(this));

			/** タイプ固有のステートの追加 */
			switch (m_type)
			{
			case EnChildPenguinType::Naughty:
				// AddState<NaughtyRampageState>(this); // 実装時に追加
				break;
			default:
				break;
			}

			/** 初期ステートの設定 */
			m_currentState = FindState(PenguinIdleState::ID());
			m_currentState->Enter();
		}


		const ChildPenguinStatus* ChildPenguinStateMachine::GetChildPenguinStatus() const
		{
			return m_ownerActor->GetStatus<ChildPenguinStatus>();
		}


		const PenguinStatus* ChildPenguinStateMachine::GetPenguinStatus() const
		{
			return GetChildPenguinStatus();
		}


		void ChildPenguinStateMachine::Damage()
		{
			const_cast<ChildPenguinStatus*>(GetChildPenguinStatus())->Damage();
		}


		void ChildPenguinStateMachine::OnDead()
		{
			// ChildPenguinManagerからの削除とdeleteを行う。
			ChildPenguinManager::GetInstance()->RemoveAndDestroy(m_ownerChildPenguin);
		}


		core::IState* ChildPenguinStateMachine::GetChangeState()
		{
         // 1. システム・環境系の判定（ダメージ、死亡、水泳など）
			// 死亡中状態の維持
			if (IsEqualCurrentState(PenguinDiyingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinDiyingState::ID());
				}
				return FindState(PenguinDeadState::ID());
			}

			// 死亡判定
			if (GetChildPenguinStatus()->IsDead())
			{
				return FindState(PenguinDiyingState::ID());
			}

			// 被弾判定
			if (CanChangeDamagedState())
			{
				return FindState(PenguinDamagedState::ID());
			}

			// 泳ぎ判定
			if (IsEqualCurrentState(PenguinSwimmingState::ID()))
			{
				if (!IsOnGround()) return FindState(PenguinSwimmingState::ID());
			}
			else if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}

			// 2. アクション系の判定（スライド、ジャンプ、移動など）
			// スライド開始アニメ中→スライド
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))
			{
				if (CanChangeSlidingState()) return FindState(PenguinSlidingState::ID());
			}

			// スライド終了アニメ継続中
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				if (!IsFinishedSlideEndState()) return FindState(PenguinSlideEndState::ID());
			}

			// スライド中の維持/終了
			if (IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState()) return FindState(PenguinSlidingState::ID());
				else return FindState(PenguinSlideEndState::ID());
			}

			// スライド開始判定
			if (CanChangeSlideStartState())
			{
				return FindState(PenguinSlideStartState::ID());
			}

			// ジャンプ判定（または空中にいる場合）
			if (CanChangeJumpState() || !IsOnGround())
			{
				return FindState(PenguinJumpState::ID());
			}

			// ダッシュ判定
			if (CanChangeRunState())
			{
				return FindState(PenguinRunState::ID());
			}

			// 通常移動判定
			if (CanChangeMoveState())
			{
				return FindState(PenguinSneakState::ID());
			}

			// その他（固有ステートなど）
			if (auto* typeState = GetTypeSpecificChangeState())
			{
				return typeState;
			}

			// どれにも当てはまらなければ待機
			return FindState(PenguinIdleState::ID());
		}
	}
}