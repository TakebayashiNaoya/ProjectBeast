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
			, m_isFollow(false)
			, m_isWait(false)
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
			AddState<PenguinDivingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSwimmingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinClimbStartState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinClimbingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinClimbEndState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDamagedState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDiyingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDeadState>(static_cast<PenguinStateMachine*>(this));

			/** タイプ固有のステートの追加 */
			switch (m_type)
			{
			case EnChildPenguinType::naughty:
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
			m_isDive = isDive;
			m_isSeparateWater = isSeparateWater;
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

			// 登り終了状態
			if (IsEqualCurrentState(PenguinClimbEndState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbEndState::ID());
				}
				// アニメーション終了後はIdleへ
			}

			// 登り中状態
			if (IsEqualCurrentState(PenguinClimbingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbingState::ID());
				}
				return FindState(PenguinClimbEndState::ID());
			}

			// 登り開始状態
			if (IsEqualCurrentState(PenguinClimbStartState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbStartState::ID());
				}
				return FindState(PenguinClimbingState::ID());
			}

			// 離水判定
			if (CanChangeSeparateWaterState())
			{
				return FindState(PenguinClimbStartState::ID());
			}

			// 泳ぎ判定
			if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}

			// 飛び込み状態の維持
			if (IsEqualCurrentState(PenguinDivingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinDivingState::ID());
				}
				return FindState(PenguinSwimmingState::ID());
			}

			// 飛び込み判定
			if (CanChangeDivingState())
			{
				return FindState(PenguinDivingState::ID());
			}

			// スライド終了状態
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinSlideEndState::ID());
				}
				// アニメーション終了後は次の判定へ
			}

			// スライド中状態
			if (IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
				return FindState(PenguinSlideEndState::ID());
			}

			// スライド開始状態
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))
			{
				if (CanChangeSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
				return FindState(PenguinSlideStartState::ID());
			}

			// スライド開始判定
			if (CanChangeSlideStartState())
			{
				return FindState(PenguinSlideStartState::ID());
			}

			// ダッシュ判定
			if (CanChangeRunState())
			{
				return FindState(PenguinRunState::ID());
			}

			// 歩行判定
			if (CanChangeWalkState())
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