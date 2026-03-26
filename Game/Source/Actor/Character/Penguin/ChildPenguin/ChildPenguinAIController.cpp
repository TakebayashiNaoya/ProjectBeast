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


		Vector3 ChildPenguinAIController::CalculateDirectionToTarget(const Vector3& targetPos) const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			Vector3 direction = targetPos - childPos;
			direction.y = 0.0f;
			direction.Normalize();
			return direction;
		}


		float ChildPenguinAIController::GetDistanceToTarget(const Vector3& targetPos) const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			Vector3 diff = targetPos - childPos;
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
			bool isFollowCommand = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;
			const float distanceToDaddy = GetDistanceToDaddy();

			if (isFollowCommand)
			{
				// 親の命令範囲に入ったら、隊列に参加する
				if (!m_isFollowing && distanceToDaddy <= JOIN_DISTANCE)
				{
					manager->AddFollower(m_owner);
					m_isFollowing = true;
				}

				if (m_isFollowing)
				{
					// 陣形の自分の担当ポジション座標を取得
					Vector3 targetPos = m_owner->GetFormationTargetPosition();
					float distanceToTarget = GetDistanceToTarget(targetPos);

					// 担当ポジションに十分近づいていなければ移動する
					if (distanceToTarget > STOP_DISTANCE)
					{
						const Vector3 moveDirection = CalculateDirectionToTarget(targetPos);
						const bool isDash = distanceToTarget > DASH_DISTANCE;
						m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
					}
					else
					{
						// ポジションに到着したら停止
						m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
					}
				}
				else
				{
					// まだ命令範囲外で隊列に入っていない場合は、親の方へ向かう
					const Vector3 moveDirection = CalculateDirectionToDaddy();
					const bool isDash = distanceToDaddy > DASH_DISTANCE;
					m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
				}
			}
			else
			{
				// 待機命令などが出た場合、隊列から離脱して停止する
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
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
			bool isFollowCommand = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;
			const float distanceToDaddy = GetDistanceToDaddy();

			// 追従命令中、または待機命令でも親から離れすぎた場合は強制追従
			if (isFollowCommand || (!isFollowCommand && distanceToDaddy > BREAK_AWAY_DISTANCE))
			{
				if (!m_isFollowing && distanceToDaddy <= JOIN_DISTANCE)
				{
					manager->AddFollower(m_owner);
					m_isFollowing = true;
				}

				if (m_isFollowing)
				{
					Vector3 targetPos = m_owner->GetFormationTargetPosition();
					float distanceToTarget = GetDistanceToTarget(targetPos);

					if (distanceToTarget > STOP_DISTANCE)
					{
						const Vector3 moveDirection = CalculateDirectionToTarget(targetPos);
						const bool isDash = distanceToTarget > DASH_DISTANCE;
						m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
					}
					else
					{
						m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
					}
				}
				else
				{
					const Vector3 moveDirection = CalculateDirectionToDaddy();
					const bool isDash = distanceToDaddy > DASH_DISTANCE;
					m_stateMachine->AIControllerInput(moveDirection, isDash, false, false, false, false);
				}
			}
			else
			{
				// 待機命令中で、かつ親との距離がBREAK_AWAY_DISTANCE以内の場合はおとなしく待機
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
			}
		}
	}
}
