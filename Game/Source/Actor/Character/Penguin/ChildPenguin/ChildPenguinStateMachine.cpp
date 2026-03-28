/**
 * @file ChildPenguinStateMachine.cpp
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
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


		void ChildPenguinStateMachine::AIControllerInput(const Vector3& moveDirection, bool isDash, bool isJump, bool isSlide, bool isDive, bool isSeparateWater)
		{
			m_moveDirection = moveDirection;
			m_isDash = isDash;
			m_isJump = isJump;
			m_isSlide = isSlide;
			m_isSwimming = IsInWater();
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


		core::IState* ChildPenguinStateMachine::GetChangeState()
		{
			// ジャンプ判定
			if (CanChangeJumpState())
			{
				return FindState(PenguinJumpState::ID());
			}

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
			if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}

			// スライド開始状態
			// ※ SlideStart を経由せず直接 Sliding へ遷移する。
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))
			{
				return FindState(PenguinSlidingState::ID());
			}

			// スライド中状態
			// ※ スライドを終了するとき SlideEnd を経由せず次の判定へ直接遷移する。
			if (IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
				// SlideEnd をスキップ → 次の判定（Run / Sneak / Idle）へ落とす
			}

			// スライド開始判定
			// ※ SlideStart を経由せず直接 Sliding へ遷移する。
			if (CanChangeSlideStartState())
			{
				return FindState(PenguinSlidingState::ID());
			}

			// ダッシュ判定
			if (CanChangeRunState())
			{
				return FindState(PenguinRunState::ID());
			}

			// 歩行判定
			if (CanChangeMoveState())
			{
				return FindState(PenguinSneakState::ID());
			}

			// タイプ固有のステート遷移
			core::IState* typeSpecificState = GetTypeSpecificChangeState();
			if (typeSpecificState != nullptr)
			{
				return typeSpecificState;
			}

			// デフォルトは待機
			return FindState(PenguinIdleState::ID());
		}
	}
}