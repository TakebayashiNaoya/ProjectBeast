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


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin)
			: PenguinStateMachine(ownerDaddyPenguin)
			, m_ownerDaddyPenguin(ownerDaddyPenguin)
			, m_isFollowCommand(false)
			, m_isWaitCommand(false)
		{
			// ステートの追加
			AddState<DaddyPenguinIdleState>(this);
			AddState<DaddyPenguinSneakState>(this);
			AddState<DaddyPenguinRunState>(this);
			AddState<DaddyPenguinJumpState>(this);
			AddState<DaddyPenguinSlideStartState>(this);
			AddState<DaddyPenguinSlidingState>(this);
			AddState<DaddyPenguinSlideEndState>(this);
			AddState<DaddyPenguinCommandShoutState>(this);

			// 初期ステートの設定
			m_currentState = FindState(DaddyPenguinIdleState::ID());
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
		}


		const DaddyPenguinStatus* DaddyPenguinStateMachine::GetDaddyPenguinStatus() const
		{
			return m_ownerActor->GetStatus<DaddyPenguinStatus>();
		}


		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			if (CanChangeJumpState())
			{
				return FindState(DaddyPenguinJumpState::ID());
			}


			if (IsEqualCurrentState(DaddyPenguinCommandShoutState::ID())
				&& IsPlayingAnimation())
			{
				// 追従命令状態でアニメーション再生中なら維持する
				return FindState(DaddyPenguinCommandShoutState::ID());
			}


			if (CanChangeCommandState())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}





			/** スライド終わりのアニメーション再生中なら維持する */
			if (IsEqualCurrentState(DaddyPenguinSlideEndState::ID()))
			{
				if (!IsFinishedSlideEndState())
				{
					return FindState(DaddyPenguinSlideEndState::ID());
				}
			}


			/** スライド中なら、スライドを維持するか判断する */
			if (IsEqualCurrentState(DaddyPenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState())
				{
					return FindState(DaddyPenguinSlidingState::ID());
				}
				else
				{
					return FindState(DaddyPenguinSlideEndState::ID());
				}
			}


			/** スライド開始中ならアニメーションが終わるまで維持し、終わるとスライディングステートへ */
			if (IsEqualCurrentState(DaddyPenguinSlideStartState::ID()))
			{
				if (CanChangeSlidingState())
				{
					return FindState(DaddyPenguinSlidingState::ID());
				}
			}


			/** スライドを始められるならスライド開始状態へ */
			if (CanChangeSlideStartState())
			{
				return FindState(DaddyPenguinSlideStartState::ID());
			}


			/** ダッシュ入力があり、移動入力があればダッシュ状態へ */
			if (CanChangeRunState())
			{
				return FindState(DaddyPenguinRunState::ID());
			}


			/** 移動入力があればスニーク状態へ */
			if (CanChangeWalkState())
			{
				return FindState(DaddyPenguinSneakState::ID());
			}


			/** 当てはまらなければ待機状態へ */
			return FindState(DaddyPenguinIdleState::ID());
		}
	}
}