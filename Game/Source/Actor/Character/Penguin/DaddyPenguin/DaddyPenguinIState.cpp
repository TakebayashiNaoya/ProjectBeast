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




		/************************************/


		void DaddyPenguinDivingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleShake);
		}


		void DaddyPenguinDivingState::Update()
		{}


		void DaddyPenguinDivingState::Exit()
		{}


		DaddyPenguinDivingState::DaddyPenguinDivingState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinSwimmingState::Enter()
		{
			const float moveSpeed = m_owner->GetDaddyPenguinStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveSwim);
		}


		void DaddyPenguinSwimmingState::Update()
		{
			m_owner->Move();
		}


		void DaddyPenguinSwimmingState::Exit()
		{}


		DaddyPenguinSwimmingState::DaddyPenguinSwimmingState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinClimbStartState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchBegin);
		}

		void DaddyPenguinClimbStartState::Update()
		{}


		void DaddyPenguinClimbStartState::Exit()
		{}


		DaddyPenguinClimbStartState::DaddyPenguinClimbStartState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinClimbingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchFlapingWingsQuickly);
		}


		void DaddyPenguinClimbingState::Update()
		{}


		void DaddyPenguinClimbingState::Exit()
		{}


		DaddyPenguinClimbingState::DaddyPenguinClimbingState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinClimbEndState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchEnd);
		}


		void DaddyPenguinClimbEndState::Update()
		{}


		void DaddyPenguinClimbEndState::Exit()
		{}


		DaddyPenguinClimbEndState::DaddyPenguinClimbEndState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinDamagedState::Enter()
		{
			m_owner->GetDaddyPenguinStatus()->Damage();
		}


		void DaddyPenguinDamagedState::Update()
		{}


		void DaddyPenguinDamagedState::Exit()
		{}


		DaddyPenguinDamagedState::DaddyPenguinDamagedState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinDiyingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::DeathFaceDown);
		}


		void DaddyPenguinDiyingState::Update()
		{}


		void DaddyPenguinDiyingState::Exit()
		{}


		DaddyPenguinDiyingState::DaddyPenguinDiyingState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinDeadState::Enter()
		{
			m_owner->SetActive(false);
		}


		void DaddyPenguinDeadState::Update()
		{}


		void DaddyPenguinDeadState::Exit()
		{}


		DaddyPenguinDeadState::DaddyPenguinDeadState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}
	}
}