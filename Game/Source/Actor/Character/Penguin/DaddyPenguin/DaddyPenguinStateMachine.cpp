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
		{
			// ステートの追加
			AddState<DaddyPenguinIdleState>(this);
			AddState<DaddyPenguinSneakState>(this);
			AddState<DaddyPenguinRunState>(this);
			AddState<DaddyPenguinJumpState>(this);
			AddState<DaddyPenguinSlideStartState>(this);
			AddState<DaddyPenguinSlidingState>(this);
			AddState<DaddyPenguinSlideEndState>(this);

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
			m_isSlide = g_pad[0]->IsPress(enButtonX);
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
			if (CanChangeRunState())
			{
				return FindState(DaddyPenguinRunState::ID());
			}
			if (CanChangeWalkState())
			{
				return FindState(DaddyPenguinSneakState::ID());
			}
			return FindState(DaddyPenguinIdleState::ID());
		}
	}
}