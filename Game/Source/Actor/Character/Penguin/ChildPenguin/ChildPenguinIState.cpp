/**
 * @file ChildPenguinIState.cpp
 * @brief 子ペンギンのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinIState.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"


namespace app
{
	namespace actor
	{

		ChildPenguinIState::ChildPenguinIState(ChildPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void ChildPenguinIdleState::Enter()
		{}


		void ChildPenguinIdleState::Update()
		{}


		void ChildPenguinIdleState::Exit()
		{}


		ChildPenguinIdleState::ChildPenguinIdleState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}




		/************************************/


		void ChildPenguinMoveState::Enter()
		{}


		void ChildPenguinMoveState::Update()
		{}


		void ChildPenguinMoveState::Exit()
		{}


		ChildPenguinMoveState::ChildPenguinMoveState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}




		/************************************/


		void ChildPenguinFollowState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleFlapingWings);
		}


		void ChildPenguinFollowState::Update()
		{}


		void ChildPenguinFollowState::Exit()
		{}


		ChildPenguinFollowState::ChildPenguinFollowState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}




		/************************************/


		void ChildPenguinWaitState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleLoocAround);
		}


		void ChildPenguinWaitState::Update()
		{}


		void ChildPenguinWaitState::Exit()
		{}


		ChildPenguinWaitState::ChildPenguinWaitState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}
	}
}