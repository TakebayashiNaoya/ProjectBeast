/**
 * @file PenguinStateMachine.cpp
 * @brief ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinBase.h"
#include "PenguinStateMachine.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			/** 重力 */
			constexpr float GRAVITY = -9.8f;
		}


		void PenguinStateMachine::Jump()
		{
			m_currentJumpPower += GRAVITY;
			const float jumpPower = m_jumpPower + m_currentJumpPower;
			Vector3 jumpVector = Vector3::Up * jumpPower;
			Vector3 nextPosition = m_ownerActor->GetTransform().m_position + jumpVector;

			m_ownerActor->SetPosition(nextPosition);

			if (nextPosition.y < 0.0f)
			{
				nextPosition.y = 0.0f;
			}
		}


		PenguinStateMachine::PenguinStateMachine(PenguinBase* ownerPenguinBase)
			: CharacterStateMachine(ownerPenguinBase)
			, m_ownerPenguinBase(ownerPenguinBase)
			, m_currentJumpPower(0.0f)
			, m_jumpPower(0.0f)
		{}


		core::IState* PenguinStateMachine::GetChangeState()
		{
			return nullptr;
		}
	}
}