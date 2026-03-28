/**
 * @file ChildPenguinAIController.cpp
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include <random>
#include <algorithm>
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ChildPenguinTypes.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/**
			 * @brief 乱数エンジン（起動時に一度だけシード初期化）
			 */
			std::mt19937& GetRandomEngine()
			{
				static std::mt19937 engine(std::random_device{}());
				return engine;
			}

			/**
			 * @brief 範囲 [r.min, r.max] から一様乱数を生成する
			 * @param r 範囲
			 * @return 生成された乱数値
			 */
			float RollRange(const MasterChildPenguinParameter::Range& r)
			{
				std::uniform_real_distribution<float> dist(r.min, r.max);
				return dist(GetRandomEngine());
			}

			/**
			 * @brief タイプに対応する TypeData を取得する
			 * @details LoadParameter はJSON配列の要素ごとに別インスタンスを生成するため、
			 * インデックスはタイプの値と対応する。
			 * @param type 子ペンギンのタイプ
			 * @return タイプ別パラメーター
			 */
			const MasterChildPenguinParameter::ChildPenguinTypeData& GetTypeData(EnChildPenguinType type)
			{
				const int index = static_cast<int>(type);
				const auto* param = core::ParameterManager::Get()->GetParameter<MasterChildPenguinParameter>(index);
				return param->typeData[index];
			}
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ChildPenguinAIController（基底クラス）
		//--------------------------------------------------------------

		ChildPenguinAIController::ChildPenguinAIController(ChildPenguin* owner, EnChildPenguinType type)
			: m_owner(owner)
			, m_stateMachine(owner->GetStateMachine())
		{
			const auto& td = GetTypeData(type);

			/** 速度系個体値を決定してStatusに反映する */
			const float runSpeed = RollRange(td.runSpeed);
			const float swimSpeed = RollRange(td.swimSpeed);
			const float sneakSpeed = RollRange(td.sneakSpeed);
			const float slideSpeed = RollRange(td.slideSpeed);
			const float jumpPower = RollRange(td.jumpPower);
			owner->GetStatus<ChildPenguinStatus>()->SetIndividualValues(
				runSpeed, swimSpeed, sneakSpeed, slideSpeed, jumpPower);

			/** 距離系個体値を決定する */
			m_stopDistance = RollRange(td.stopDistance);
			m_walkDistance = RollRange(td.walkDistance);
			m_runDistance = RollRange(td.runDistance);
			m_joinDistance = RollRange(td.joinDistance);
			m_giveUpDistance = RollRange(td.giveUpDistance);

			/** 制約補正：stopDistance < walkDistance < runDistance < joinDistance < giveUpDistance */
			m_walkDistance = max(m_walkDistance, m_stopDistance + 1.0f);
			m_runDistance = max(m_runDistance, m_walkDistance + 1.0f);
			m_joinDistance = max(m_joinDistance, m_runDistance + 1.0f);
			m_giveUpDistance = max(m_giveUpDistance, m_joinDistance + 1.0f);
		}


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


		void ChildPenguinAIController::BuildInput()
		{
			const Vector3 targetPos = m_owner->GetFormationTargetPosition();
			const float   distToTarget = GetDistanceToTarget(targetPos);

			/**
			 * ヒステリシスを考慮したフェーズ遷移
			 *
			 * フェーズを「上げる閾値」と「下げる閾値」を非対称にする。
			 * 上げる : distance > threshold               （外に出たらすぐ上げる）
			 * 下げる : distance <= threshold - HYSTERESIS （十分内側に入ったら下げる）
			 */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget > m_stopDistance) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Walk:
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget <= m_stopDistance - HYSTERESIS) { m_movePhase = MovePhase::Stop; }
				break;

			case MovePhase::Run:
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget <= m_walkDistance - HYSTERESIS) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Slide:
				if (distToTarget <= m_runDistance - HYSTERESIS) { m_movePhase = MovePhase::Run; }
				break;
			}

			/**
			 * 移動方向（常に正規化済みベクトルを渡す）
			 *
			 * Move() は moveDirection * moveSpeed で移動量を計算するため、
			 * moveDirection のスケールを変えると速度が変わってしまう。
			 * 速度の調整は SetMoveSpeed() 経由のみで行う。
			 */
			const Vector3 moveDirection = CalculateDirectionToTarget(targetPos);

			/** フェーズに応じてAIControllerInputを組み立てる */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				break;

			case MovePhase::Walk:
				/** 歩き：isDash=false, isSlide=false */
				m_stateMachine->AIControllerInput(moveDirection, false, false, false, false, false);
				break;

			case MovePhase::Run:
				/** 走り：isDash=true, isSlide=false */
				m_stateMachine->AIControllerInput(moveDirection, true, false, false, false, false);
				break;

			case MovePhase::Slide:
				/** 滑り：isDash=true, isSlide=true */
				m_stateMachine->AIControllerInput(moveDirection, true, false, true, false, false);
				break;
			}
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// SeriousChildPenguinAI（まじめペンギン）
		//--------------------------------------------------------------

		SeriousChildPenguinAI::SeriousChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Serious)
		{}


		void SeriousChildPenguinAI::Update()
		{
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 待機命令のとき */
			if (manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Wait)
			{
				/** 隊列から離脱 */
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				/** その場で待機 */
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** まだ隊列に参加していない状態で、親との距離が一定以内に入ったら参加する */
			if (!m_isFollowing && distDaddy <= m_joinDistance) {
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}

			/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
			else if (m_isFollowing && distDaddy > m_giveUpDistance) {
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 隊列に参加していない状態ならその場で待機する */
			if (!m_isFollowing) {
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}


		/**************************************************************/


		//--------------------------------------------------------------
		// ClingyChildPenguinAI（甘えん坊ペンギン）
		//--------------------------------------------------------------

		ClingyChildPenguinAI::ClingyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Clingy)
		{}


		void ClingyChildPenguinAI::Update()
		{
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** まだ隊列に参加していない状態で、親との距離が一定以内に入ったら参加する */
			if (!m_isFollowing && distDaddy <= m_joinDistance)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}

			/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
			else if (m_isFollowing && distDaddy > m_giveUpDistance)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 隊列に参加していない状態ならその場で待機する */
			if (!m_isFollowing)
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// NaughtyChildPenguinAI（やんちゃペンギン）
		//--------------------------------------------------------------

		NaughtyChildPenguinAI::NaughtyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Naughty)
		{}


		void NaughtyChildPenguinAI::Update()
		{
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 待機命令のとき */
			if (!isFollowCmd)
			{
				/** 隊列から離脱 */
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				/** その場で待機 */
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** まだ隊列に参加していない状態で、親との距離が一定以内に入ったら参加する */
			if (!m_isFollowing && distDaddy <= m_joinDistance)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}

			/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
			else if (m_isFollowing && distDaddy > m_giveUpDistance)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 隊列に参加していない状態ならその場で待機する */
			if (!m_isFollowing)
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ClumsyChildPenguinAI（おっちょこちょいペンギン）
		//--------------------------------------------------------------

		ClumsyChildPenguinAI::ClumsyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Clumsy)
		{}


		void ClumsyChildPenguinAI::Update()
		{
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 待機命令のとき */
			if (!isFollowCmd)
			{
				/** 隊列から離脱 */
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				/** その場で待機 */
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** まだ隊列に参加していない状態で、親との距離が一定以内に入ったら参加する */
			if (!m_isFollowing && distDaddy <= m_joinDistance)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}

			/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
			else if (m_isFollowing && distDaddy > m_giveUpDistance)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 隊列に参加していない状態ならその場で待機する */
			if (!m_isFollowing)
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// CaringChildPenguinAI（世話焼きペンギン）
		//--------------------------------------------------------------

		CaringChildPenguinAI::CaringChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Caring)
		{}


		void CaringChildPenguinAI::Update()
		{
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 待機命令のとき */
			if (!isFollowCmd)
			{
				/** 隊列から離脱 */
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				/** その場で待機 */
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** まだ隊列に参加していない状態で、親との距離が一定以内に入ったら参加する */
			if (!m_isFollowing && distDaddy <= m_joinDistance)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}

			/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
			else if (m_isFollowing && distDaddy > m_giveUpDistance)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 隊列に参加していない状態ならその場で待機する */
			if (!m_isFollowing)
			{
				m_stateMachine->AIControllerInput(Vector3::Zero, false, false, false, false, false);
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}
	}
}