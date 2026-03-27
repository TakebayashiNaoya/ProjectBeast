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

		//--------------------------------------------------------------
		// ChildPenguinAIController（基底クラス）
		//--------------------------------------------------------------

		ChildPenguinAIController::ChildPenguinAIController(ChildPenguin* owner)
			: m_owner(owner)
			, m_stateMachine(owner->GetStateMachine())
		{}


		Vector3 ChildPenguinAIController::CalculateDirectionToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = ChildPenguinManager::GetInstance()->GetDaddyPosition();
			Vector3 direction = daddyPos - childPos;
			direction.y = 0.0f;
			direction.Normalize();
			return direction;
		}


		float ChildPenguinAIController::GetDistanceToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = ChildPenguinManager::GetInstance()->GetDaddyPosition();
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


		void ChildPenguinAIController::BuildInput(
			float stopDistance,
			float walkDistance,
			float runDistance)
		{
			const Vector3 targetPos = m_owner->GetFormationTargetPosition();
			const float   distToTarget = GetDistanceToTarget(targetPos);

			// ---------------------------------------------------------------
			// ヒステリシスを考慮したフェーズ遷移
			//
			// フェーズを「上げる閾値」と「下げる閾値」を非対称にする。
			//   上げる : distance > threshold               （外に出たらすぐ上げる）
			//   下げる : distance <= threshold - HYSTERESIS （十分内側に入ったら下げる）
			// ---------------------------------------------------------------
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				if (distToTarget > runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget > stopDistance) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Walk:
				if (distToTarget > runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget <= stopDistance - HYSTERESIS) { m_movePhase = MovePhase::Stop; }
				break;

			case MovePhase::Run:
				if (distToTarget > runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget <= walkDistance - HYSTERESIS) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Slide:
				if (distToTarget <= runDistance - HYSTERESIS) { m_movePhase = MovePhase::Run; }
				break;
			}

			// ---------------------------------------------------------------
			// 移動方向（常に正規化済みベクトルを渡す）
			//
			// Move() は moveDirection * moveSpeed で移動量を計算するため、
			// moveDirection のスケールを変えると速度が変わってしまう。
			// 速度の調整は SetMoveSpeed() 経由のみで行う。
			// ---------------------------------------------------------------
			const Vector3 moveDirection = CalculateDirectionToTarget(targetPos);

			// ---------------------------------------------------------------
			// フェーズに応じてAIControllerInputを組み立てる
			// ---------------------------------------------------------------
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				break;

			case MovePhase::Walk:
				// 歩き：isDash=false, isSlide=false
				m_stateMachine->AIControllerInput(moveDirection, false, false, false, false, false);
				break;

			case MovePhase::Run:
				// 走り：isDash=true, isSlide=false
				m_stateMachine->AIControllerInput(moveDirection, true, false, false, false, false);
				break;

			case MovePhase::Slide:
				// 滑り：isDash=true, isSlide=true
				m_stateMachine->AIControllerInput(moveDirection, true, false, true, false, false);
				break;
			}
		}


		//--------------------------------------------------------------
		// SeriousChildPenguinAI
		//--------------------------------------------------------------

		SeriousChildPenguinAI::SeriousChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner)
		{}


		void SeriousChildPenguinAI::Update()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;
			const float distDaddy = GetDistanceToDaddy();

			if (!isFollowCmd)
			{
				// 待機命令：隊列から離脱して停止
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			// ---------------------------------------------------------------
			// 追従命令中の隊列参加・離脱管理
			//
			// 未参加 → JOIN_DISTANCE     以内に入ったら参加
			// 参加中 → GIVE_UP_DISTANCE  を超えたら離脱してその場で待機
			//          再び JOIN_DISTANCE 以内に入ると追従を再開する
			// ---------------------------------------------------------------
			if (!m_isFollowing && distDaddy <= JOIN_DISTANCE)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}
			else if (m_isFollowing && distDaddy > GIVE_UP_DISTANCE)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			if (!m_isFollowing)
			{
				// 隊列圏外：その場で待機
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			// 隊列参加中：距離だけで移動手段を決定する
			BuildInput(STOP_DISTANCE, WALK_DISTANCE, RUN_DISTANCE);
		}


		//--------------------------------------------------------------
		// ClingyChildPenguinAI
		//--------------------------------------------------------------

		ClingyChildPenguinAI::ClingyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner)
		{}


		void ClingyChildPenguinAI::Update()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;
			const float distDaddy = GetDistanceToDaddy();

			// 待機命令中でも親から離れすぎたら強制追従
			const bool forceFollow = !isFollowCmd && distDaddy > BREAK_AWAY_DISTANCE;

			if (!isFollowCmd && !forceFollow)
			{
				// 待機命令中かつ親との距離が許容範囲内：おとなしく待機
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			// ---------------------------------------------------------------
			// 隊列参加・離脱管理（SeriousChildPenguinAI と同じ方針）
			//
			// 未参加 → JOIN_DISTANCE     以内に入ったら参加
			// 参加中 → GIVE_UP_DISTANCE  を超えたら離脱してその場で待機
			// ---------------------------------------------------------------
			if (!m_isFollowing && distDaddy <= JOIN_DISTANCE)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}
			else if (m_isFollowing && distDaddy > GIVE_UP_DISTANCE)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			if (!m_isFollowing)
			{
				// 隊列圏外：その場で待機
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			// 隊列参加中：距離だけで移動手段を決定する
			BuildInput(STOP_DISTANCE, WALK_DISTANCE, RUN_DISTANCE);
		}
	}
}