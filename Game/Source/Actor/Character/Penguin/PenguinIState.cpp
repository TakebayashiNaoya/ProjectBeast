/**
 * @file PenguinIState.cpp
 * @brief ペンギン共通のステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinIState.h"
#include "PenguinStateMachine.h"
#include "PenguinStatus.h"
#include "PenguinAnimationData.h"


namespace app
{
	namespace actor
	{

		PenguinIState::PenguinIState(PenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void PenguinIdleState::Enter()
		{
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);
		}


		void PenguinIdleState::Update()
		{}


		void PenguinIdleState::Exit()
		{}


		PenguinIdleState::PenguinIdleState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSneakState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveWalk);
		}


		void PenguinSneakState::Update()
		{
			m_owner->Move();
		}


		void PenguinSneakState::Exit()
		{}


		PenguinSneakState::PenguinSneakState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinRunState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveRun);
		}


		void PenguinRunState::Update()
		{
			m_owner->Move();
		}


		void PenguinRunState::Exit()
		{}


		PenguinRunState::PenguinRunState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinJumpState::Enter()
		{
			const float jumpPower = m_owner->GetPenguinStatus()->GetJumpPower();
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetJumpPower(jumpPower);
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::JumpWalking);
		}


		void PenguinJumpState::Update()
		{
			m_owner->Jump();
		}


		void PenguinJumpState::Exit()
		{}


		PenguinJumpState::PenguinJumpState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideStartState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::SlideStart);
		}


		void PenguinSlideStartState::Update()
		{}


		void PenguinSlideStartState::Exit()
		{}


		PenguinSlideStartState::PenguinSlideStartState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlidingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::Sliding);
		}


		void PenguinSlidingState::Update()
		{
			m_owner->Move();
		}


		void PenguinSlidingState::Exit()
		{}


		PenguinSlidingState::PenguinSlidingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideEndState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::StandUp);
		}


		void PenguinSlideEndState::Update()
		{}


		void PenguinSlideEndState::Exit()
		{}


		PenguinSlideEndState::PenguinSlideEndState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinDivingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleShake);
		}


		void PenguinDivingState::Update()
		{}


		void PenguinDivingState::Exit()
		{}


		PenguinDivingState::PenguinDivingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinSwimmingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveSwim);
		}


		void PenguinSwimmingState::Update()
		{
			m_owner->Move();
		}


		void PenguinSwimmingState::Exit()
		{}


		PenguinSwimmingState::PenguinSwimmingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinClimbStartState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchBegin);
		}


		void PenguinClimbStartState::Update()
		{}


		void PenguinClimbStartState::Exit()
		{}


		PenguinClimbStartState::PenguinClimbStartState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinClimbingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchFlapingWingsQuickly);
		}


		void PenguinClimbingState::Update()
		{}


		void PenguinClimbingState::Exit()
		{}


		PenguinClimbingState::PenguinClimbingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinClimbEndState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchEnd);
		}


		void PenguinClimbEndState::Update()
		{}


		void PenguinClimbEndState::Exit()
		{}


		PenguinClimbEndState::PenguinClimbEndState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinDamagedState::Enter()
		{
			m_owner->Damage();
		}


		void PenguinDamagedState::Update()
		{}


		void PenguinDamagedState::Exit()
		{}


		PenguinDamagedState::PenguinDamagedState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDiyingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::DeathFaceDown);
		}


		void PenguinDiyingState::Update()
		{}


		void PenguinDiyingState::Exit()
		{}


		PenguinDiyingState::PenguinDiyingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDeadState::Enter()
		{
			m_owner->SetActive(false);
		}


		void PenguinDeadState::Update()
		{}


		void PenguinDeadState::Exit()
		{}


		PenguinDeadState::PenguinDeadState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}
	}
}
