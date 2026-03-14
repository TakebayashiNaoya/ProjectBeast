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


namespace app
{
	namespace actor
	{

		DaddyPenguinIState::DaddyPenguinIState(DaddyPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void DaddyPenguinIdleState::Enter()
		{}


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
			m_owner->SetMoveDirection(Vector3::Right);
			m_owner->PlayAnimation(10);
		}


		void DaddyPenguinSneakState::Update()
		{
			m_owner->CharacterStateMachine::Move();
		}


		void DaddyPenguinSneakState::Exit()
		{}


		DaddyPenguinSneakState::DaddyPenguinSneakState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}

	}
}