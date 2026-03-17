/**
 * @file DaddyPenguinIState.cpp
 * @brief 親ペンギンのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"


namespace app
{
	namespace actor
	{

		DaddyPenguinIState::DaddyPenguinIState(DaddyPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void DaddyPenguinIdleState::Enter()
		{
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);
		}


		void DaddyPenguinIdleState::Update()
		{}


		void DaddyPenguinIdleState::Exit()
		{}


		DaddyPenguinIdleState::DaddyPenguinIdleState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinSneakState::Enter()
		{
			const float moveSpeed = m_owner->GetDaddyPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveWalk);
		}


		void DaddyPenguinSneakState::Update()
		{
			m_owner->Move();
		}


		void DaddyPenguinSneakState::Exit()
		{}


		DaddyPenguinSneakState::DaddyPenguinSneakState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinRunState::Enter()
		{
			const float moveSpeed = m_owner->GetDaddyPenguinStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveRun);
		}


		void DaddyPenguinRunState::Update()
		{
			m_owner->Move();
		}


		void DaddyPenguinRunState::Exit()
		{}


		DaddyPenguinRunState::DaddyPenguinRunState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinJumpState::Enter()
		{
			const float jumpPower = m_owner->GetDaddyPenguinStatus()->GetJumpPower();
			const float moveSpeed = m_owner->GetDaddyPenguinStatus()->GetSneakSpeed();
			m_owner->SetJumpPower(jumpPower);
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::JumpWalking);
		}


		void DaddyPenguinJumpState::Update()
		{
			m_owner->Jump();
		}


		void DaddyPenguinJumpState::Exit()
		{}


		DaddyPenguinJumpState::DaddyPenguinJumpState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinSlideStartState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::SlideStart);
		}


		void DaddyPenguinSlideStartState::Update()
		{}


		void DaddyPenguinSlideStartState::Exit()
		{}


		DaddyPenguinSlideStartState::DaddyPenguinSlideStartState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinSlidingState::Enter()
		{
			const float moveSpeed = m_owner->GetDaddyPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::Sliding);
		}


		void DaddyPenguinSlidingState::Update()
		{
			m_owner->Move();
		}


		void DaddyPenguinSlidingState::Exit()
		{}


		DaddyPenguinSlidingState::DaddyPenguinSlidingState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinSlideEndState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::StandUp);
		}


		void DaddyPenguinSlideEndState::Update()
		{}


		void DaddyPenguinSlideEndState::Exit()
		{}


		DaddyPenguinSlideEndState::DaddyPenguinSlideEndState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinCommandShoutState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);
		}


		void DaddyPenguinCommandShoutState::Update()
		{}


		void DaddyPenguinCommandShoutState::Exit()
		{}


		DaddyPenguinCommandShoutState::DaddyPenguinCommandShoutState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}
	}
}