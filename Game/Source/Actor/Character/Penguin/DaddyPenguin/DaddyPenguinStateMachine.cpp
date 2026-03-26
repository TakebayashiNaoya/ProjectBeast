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
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin)
			: PenguinStateMachine(ownerDaddyPenguin)
			, m_ownerDaddyPenguin(ownerDaddyPenguin)
			, m_isFollowCommand(false)
			, m_isWaitCommand(false)
			, m_isWin(false)
			, m_isLose(false)
		{
			// 共通ステートの追加
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

			// Daddy固有のステートの追加
			AddState<DaddyPenguinCommandShoutState>(this);
			AddState<DaddyPenguinWinState>(this);
			AddState<DaddyPenguinLoseState>(this);

			// 初期ステートの設定
			m_currentState = FindState(PenguinIdleState::ID());
			m_currentState->Enter();
		}


		void DaddyPenguinStateMachine::PlayerControllerInput()
		{
			m_moveDirection.x = g_pad[0]->GetLStickXF();
			m_moveDirection.z = g_pad[0]->GetLStickYF();
			m_moveDirection.y = 0.0f;

			m_isDash = g_pad[0]->IsPress(enButtonB);
			m_isJump = g_pad[0]->IsTrigger(enButtonA);
			m_isSlide = g_pad[0]->IsPress(enButtonRB3);

			m_isFollowCommand = g_pad[0]->IsTrigger(enButtonLB1);
			m_isWaitCommand = g_pad[0]->IsTrigger(enButtonRB1);
			m_isSwimming = g_pad[0]->IsPress(enButtonX);
			m_isDive = m_isSeparateWater = g_pad[0]->IsTrigger(enButtonY);
			m_isDamaged = g_pad[0]->IsTrigger(enButtonLB3);

			m_isWin = g_pad[0]->IsTrigger(enButtonLB2);
			m_isLose = g_pad[0]->IsTrigger(enButtonRB2);
		}


		DaddyPenguinStatus* DaddyPenguinStateMachine::GetDaddyPenguinStatus() const
		{
			return m_ownerActor->GetStatus<DaddyPenguinStatus>();
		}


		const PenguinStatus* DaddyPenguinStateMachine::GetPenguinStatus() const
		{
			return GetDaddyPenguinStatus();
		}


		void DaddyPenguinStateMachine::Damage()
		{
			GetDaddyPenguinStatus()->Damage();
		}


		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			/** 命令中なら命令ステートへ */
			if (CanChangeCommandState())
			{
				if (ChildPenguinManager::GetInstance()->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow)
				{
					ChildPenguinManager::GetInstance()->SetCommand(ChildPenguinManager::EnPenguinCommand::Wait);
				}
				else if (ChildPenguinManager::GetInstance()->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow)
				{
					ChildPenguinManager::GetInstance()->SetCommand(ChildPenguinManager::EnPenguinCommand::Follow);
				}
				return FindState(DaddyPenguinCommandShoutState::ID());
			}


			if (IsEqualCurrentState(DaddyPenguinWinState::ID()))
			{
				return FindState(DaddyPenguinWinState::ID());
			}



			if (m_isWin)
			{
				return FindState(DaddyPenguinWinState::ID());
			}


			if (IsEqualCurrentState(DaddyPenguinLoseState::ID()))
			{
				return FindState(DaddyPenguinLoseState::ID());
			}


			if (m_isLose)
			{
				return FindState(DaddyPenguinLoseState::ID());
			}


			if (CanChangeJumpState())
			{
				return FindState(PenguinJumpState::ID());
			}


			/** 死亡ステート中、アニメーション再生中であれば継続 */
			if (IsEqualCurrentState(PenguinDiyingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinDiyingState::ID());
				}
				else
				{
					return FindState(PenguinDeadState::ID());
				}
			}



			if (m_ownerDaddyPenguin->GetStatus<DaddyPenguinStatus>()->IsDead())
			{
				return FindState(PenguinDiyingState::ID());
			}


			if (m_isDamaged)
			{
				return FindState(PenguinDamagedState::ID());
			}


			/** 登り終わり中だったら */
			if (IsEqualCurrentState(PenguinClimbEndState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbEndState::ID());
				}
				else
				{
					return FindState(PenguinIdleState::ID());
				}
			}


			/** 登り中だったら */
			if (IsEqualCurrentState(PenguinClimbingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbingState::ID());
				}
				else
				{
					return FindState(PenguinClimbEndState::ID());
				}
			}


			/** 登り始まり中だったら */
			if (IsEqualCurrentState(PenguinClimbStartState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinClimbStartState::ID());

				}
				else
				{
					return FindState(PenguinClimbingState::ID());
				}
			}


			/** 水からなられる準備ができたら */
			if (CanChangeSeparateWaterState())
			{
				return FindState(PenguinClimbStartState::ID());
			}




			/** 泳ぐステートに変更可能なら */
			if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}



			if (IsEqualCurrentState(PenguinDivingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinDivingState::ID());
				}
				else
				{
					return FindState(PenguinSwimmingState::ID());
				}
			}


			if (CanChangeDivingState())
			{
				return FindState(PenguinDivingState::ID());
			}



			/** 命令中なら維持する */
			if (IsEqualCurrentState(DaddyPenguinCommandShoutState::ID())
				&& IsPlayingAnimation())
			{
				// 命令状態でアニメーション再生中なら維持する
				return FindState(DaddyPenguinCommandShoutState::ID());
			}


			/** 命令状態へなれるなら命令状態へ */
			if (CanChangeCommandState())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}





			/** スライド終わりのアニメーション再生中なら維持する */
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				if (!IsFinishedSlideEndState())
				{
					return FindState(PenguinSlideEndState::ID());
				}
			}


			/** スライド中なら、スライドを維持するか判断する */
			if (IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
				else
				{
					return FindState(PenguinSlideEndState::ID());
				}
			}


			/** スライド開始中ならアニメーションが終わるまで維持し、終わるとスライディングステートへ */
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))
			{
				if (CanChangeSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
			}


			/** スライドを始められるならスライド開始状態へ */
			if (CanChangeSlideStartState())
			{
				return FindState(PenguinSlideStartState::ID());
			}


			/** ダッシュ入力があり、移動入力があればダッシュ状態へ */
			if (CanChangeRunState())
			{
				return FindState(PenguinRunState::ID());
			}


			/** 移動入力があればスニーク状態へ */
			if (CanChangeWalkState())
			{
				return FindState(PenguinSneakState::ID());
			}


			/** 当てはまらなければ待機状態へ */
			return FindState(PenguinIdleState::ID());
		}
	}
}