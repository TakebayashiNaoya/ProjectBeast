/**
 * @file ChildPenguinAIController.cpp
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"


namespace app
{
	namespace actor
	{

		ChildPenguinAIController::ChildPenguinAIController(ChildPenguin* owner, DaddyPenguin* daddyPenguin)
			: m_owner(owner)
			, m_daddyPenguin(daddyPenguin)
			, m_stateMachine(owner->GetStateMachine())
		{}


		Vector3 ChildPenguinAIController::CalculateDirectionToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;
			Vector3 direction = daddyPos - childPos;
			direction.y = 0.0f;
			direction.Normalize();
			return direction;
		}


		float ChildPenguinAIController::GetDistanceToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;
			Vector3 diff = daddyPos - childPos;
			diff.y = 0.0f;
			return diff.Length();
		}




		/************************************/


		SeriousChildPenguinAI::SeriousChildPenguinAI(ChildPenguin* owner, DaddyPenguin* daddyPenguin)
			: ChildPenguinAIController(owner, daddyPenguin)
		{}


		void SeriousChildPenguinAI::Update()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const float distance = GetDistanceToDaddy();

			bool isFollowCommand = ChildPenguinManager::GetInstance()->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			if (isFollowCommand && distance > FOLLOW_DISTANCE)
			{
				const Vector3 moveDirection = CalculateDirectionToDaddy();
				const bool isDash = distance > DASH_DISTANCE;
				m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
			}
			else
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
			}
		}




		/************************************/


		ClingyChildPenguinAI::ClingyChildPenguinAI(ChildPenguin* owner, DaddyPenguin* daddyPenguin)
			: ChildPenguinAIController(owner, daddyPenguin)
		{}


		void ClingyChildPenguinAI::Update()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const float distance = GetDistanceToDaddy();

			bool isFollowCommand = ChildPenguinManager::GetInstance()->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			// 追従命令中、または待機命令中でも離れすぎた場合は追従（甘えん坊固有）
			if (isFollowCommand && distance > FOLLOW_DISTANCE)
			{
				const Vector3 moveDirection = CalculateDirectionToDaddy();
				const bool isDash = distance > DASH_DISTANCE;
				m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
			}
			else
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
			}
		}
	}
}
