/**
 * @file ChildPenguinAIController.cpp
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷、竹林
 */
#include "stdafx.h"

#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ChildPenguinTypes.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Manager/IglooManager.h"
#include <algorithm>
#include <random>



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
			 * @brief [0, 1) の一様乱数を生成する
			 * @return 生成された乱数値
			 */
			float RollUnit()
			{
				std::uniform_real_distribution<float> dist(0.0f, 1.0f);
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
			BuildInputToTarget(targetPos);
		}


		void ChildPenguinAIController::BuildInputToTarget(const Vector3& targetPos)
		{
			const float distToTarget = GetDistanceToTarget(targetPos);


			// 目標付近（m_stopDistance + HYSTERESIS 以内）にいて、かつ物理的にほぼ停止しているなら、
			// 目標にピッタリ到達していなくても強制的に Stop フェーズにする。
			if (m_movePhase != MovePhase::Stop && distToTarget <= m_stopDistance + HYSTERESIS)
			{
				const Vector3& currentVel = m_stateMachine->GetCurrentVelocity();
				if (currentVel.LengthSq() < 0.1f) // 速度がほぼゼロ
				{
					m_movePhase = MovePhase::Stop;
				}
			}


			/**
			 * ヒステリシスと「目標到達までステートを維持する」処理を考慮したフェーズ遷移
			 * * 上げる : 設定された距離（m_walkDistance, m_runDistance）を超えたらすぐに上げる
			 * 下げる : 一度 Run や Slide になったら、途中の距離では減速せず、
			 * 所定の陣形位置（m_stopDistance付近）に到達して初めて Walk に一気に戻す
			 */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget > m_stopDistance + HYSTERESIS) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Walk:
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance) { m_movePhase = MovePhase::Run; }
				else if (distToTarget <= m_stopDistance) { m_movePhase = MovePhase::Stop; }
				break;

			case MovePhase::Run:
				/** さらに離されたら Slide へ上げる */
				if (distToTarget > m_runDistance) { m_movePhase = MovePhase::Slide; }
				/** 途中の m_walkDistance では減速せず、所定の位置（m_stopDistance）まで来たら Walk に一気に戻す */
				else if (distToTarget <= m_stopDistance) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Slide:
				/** 所定の位置（m_stopDistance）まで Slide を維持し、到着したら Walk に一気に戻す */
				if (distToTarget <= m_stopDistance) { m_movePhase = MovePhase::Walk; }
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

			float speedMultiplier = 1.0f;

			// 停止距離の2倍以内に入ったら、倍率を 1.0 から 0.0 へ徐々に下げる
			float brakeRange = m_stopDistance * 2.0f;
			if (distToTarget < brakeRange && m_movePhase != MovePhase::Stop)
			{
				// 目標に近づくほど比率が 0.0 に近づく
				float ratio = (distToTarget - m_stopDistance) / (brakeRange - m_stopDistance);
				speedMultiplier = max(0.0f, min(1.0f, ratio));
			}

			// 計算した倍率をステートマシンに渡し、物理処理(Lerp)の目標速度を落とす
			m_stateMachine->SetSpeedMultiplier(speedMultiplier);

			/** フェーズに応じてAIControllerInputを組み立てる */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
			{
				/** 停止 */
				// フェーズがStopになれば、入力がゼロになるためアニメーションもピタッと止まる
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);

				// --- 止まった後の整列処理 ---
				// 慣性が落ちてほぼ完全に止まったら、親（Daddy）の方向を向く
				const Vector3& currentVel = m_stateMachine->GetCurrentVelocity();
				if (currentVel.LengthSq() < 1.0f)
				{
					Vector3 dirToDaddy = CalculateDirectionToDaddy();
					if (dirToDaddy.LengthSq() > 0.001f)
					{
						Quaternion currentRot = m_owner->GetTransform().m_rotation;
						Quaternion targetRot;
						targetRot.SetRotationYFromDirectionXZ(dirToDaddy);

						// 一瞬で向くのではなく、Slerpでゆっくり振り向かせる
						float deltaTime = g_gameTime->GetFrameDeltaTime();
						currentRot.Slerp(6.0f * deltaTime, currentRot, targetRot);

						m_owner->SetRotation(currentRot);
					}
				}
				break;
			}

			case MovePhase::Walk:
				/** 歩き：isSneak=true, isDash=false, isSlide=false */
				m_stateMachine->SetActionInput(moveDirection, true, false, false, false);
				break;

			case MovePhase::Run:
				/** 走り：isSneak=false, isDash=true, isSlide=false */
				m_stateMachine->SetActionInput(moveDirection, false, true, false, false);
				break;

			case MovePhase::Slide:
				/** 滑り：isSneak=false, isDash=true, isSlide=true */
				m_stateMachine->SetActionInput(moveDirection, false, true, false, true);
				break;
			}
		}


		void ChildPenguinAIController::UpdateIglooEvent()
		{
			if (m_isInsideIgloo)
			{
				// ★ リーダーの助言通り、かまくらの中にいる間は「毎フレーム」座標をセットし続ける
				m_owner->GetCharacterController()->SetPosition(m_iglooTargetPos);
				m_owner->GetCharacterController()->RequestTeleport();

				// ★ 渦潮の処理と同じように、ステートマシン（モデルの見た目位置）も毎フレーム同期する！
				// これがないと見た目だけ屋根の上に押し出されてしまいます
				m_stateMachine->SetPosition(m_iglooTargetPos);

				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			Vector3 myPos = m_owner->GetTransform().m_position;

			Vector3 dirToTarget = m_iglooTargetPos - myPos;
			dirToTarget.y = 0.0f;

			// 青い円に十分近づいたらワープ発動
			if (dirToTarget.Length() < 150.0f)
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);

				// 子ペンギンの現在位置を基準に最も近いイグルーの中心座標を取得
				Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
				Vector3 insidePos = iglooPos;
				insidePos.y += 0.0f;

				// 一列に並ぶのを防ぐため、浮動小数点で円形にばらけさせる
				auto& engine = GetRandomEngine();
				std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
				std::uniform_real_distribution<float> radiusDist(0.0f, 60.0f); // かまくらの中に収まる半径

				float angleRad = angleDist(engine) * (Math::PI / 180.0f);
				float r = radiusDist(engine);

				insidePos.x += r * cosf(angleRad);
				insidePos.z += r * sinf(angleRad);

				m_iglooTargetPos = insidePos;
				m_owner->SetIglooFixedPos(m_iglooTargetPos);
				m_owner->SetInsideIgloo(true);

				// 最初のワープ時も両方の座標をセットする
				m_owner->GetCharacterController()->SetPosition(m_iglooTargetPos);
				m_owner->GetCharacterController()->RequestTeleport();
				m_stateMachine->SetPosition(m_iglooTargetPos);

				// 隊列リストからの離脱
				if (m_isFollowing)
				{
					ChildPenguinManager::GetInstance()->RemoveFollower(m_owner);
					m_isFollowing = false;
				}

				m_isInsideIgloo = true;
			}
			else
			{
				// まだ遠い場合は青い円に向かって歩く
				if (dirToTarget.LengthSq() > 0.0001f)
				{
					dirToTarget.Normalize();
				}
				m_stateMachine->SetActionInput(dirToTarget, true, false, false, false);
			}
		}


		void ChildPenguinAIController::EndEnterIglooEvent(const Vector3& exitPos)
		{
			// 1. 各種イベントフラグを解除（これで通常の追従AIに戻る）
			m_isEnterIglooMode = false;
			m_isInsideIgloo = false;
			m_owner->SetInsideIgloo(false);

			// 2. 出現座標を少しばらけさせる
			Vector3 spawnPos = exitPos;
			spawnPos.x += (float)(std::rand() % 60) - 30.0f;
			spawnPos.z += (float)(std::rand() % 60) - 30.0f;

			// 3. キャラクターコントローラーとステートマシン両方をワープ！
			m_owner->GetCharacterController()->SetPosition(spawnPos);
			m_stateMachine->SetPosition(spawnPos);
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
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
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
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
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
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
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
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 追従命令のとき：制止・登録を解除して通常追従する */
			if (isFollowCmd)
			{
				m_isRestrained = false;
				manager->UnregisterAttempting(m_owner);

				const float distDaddy = GetDistanceToDaddy();

				if (!m_isFollowing && distDaddy <= m_joinDistance)
				{
					manager->AddFollower(m_owner);
					m_isFollowing = true;
				}
				else if (m_isFollowing && distDaddy > m_giveUpDistance)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}

				if (!m_isFollowing)
				{
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
					return;
				}

				BuildInput();
				return;
			}

			/** 待機命令のとき */

			/** 世話焼きペンギンに制止されているときはその場で待機する */
			if (m_isRestrained)
			{
				/** 制止中は追従しようとしている登録を解除する */
				manager->UnregisterAttempting(m_owner);
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 待機命令中に追従しようとしていることをManagerに登録する */
			manager->RegisterAttempting(m_owner);

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
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
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
		{
			const auto& td = GetTypeData(EnChildPenguinType::Naughty);
			m_roamTriggerDistance = RollRange(td.roamTriggerDistance);
			m_roamRadius = RollRange(td.roamRadius);
		}


		void NaughtyChildPenguinAI::Update()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 世話焼きペンギンに制止されているときはその場で待機する */
			/** （命令に関わらず最優先で制止を適用する） */
			if (m_isRestrained)
			{
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 追従命令のとき */
			if (isFollowCmd)
			{
				/** joinDistance以内に入ったら徘徊を終了して隊列に参加する */
				if (distDaddy <= m_joinDistance)
				{
					/** 徘徊登録を解除する */
					manager->UnregisterRoaming(m_owner);

					if (!m_isFollowing)
					{
						manager->AddFollower(m_owner);
						m_isFollowing = true;
					}
				}

				/** すでに隊列に参加している状態で、親との距離が一定を超えたら離脱する */
				else if (m_isFollowing && distDaddy > m_giveUpDistance)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}

				/** 隊列に参加していない状態（＝まだ遠くにいる）なら徘徊を継続する */
				if (!m_isFollowing)
				{
					/** 徘徊中でなければ新しい目標を選ぶ */
					if (!manager->IsRoaming(m_owner))
					{
						manager->RegisterRoaming(m_owner);
						PickNewRoamTarget();
					}

					const float distToRoamTarget = GetDistanceToTarget(m_roamTarget);

					/** 目標地点に到達したら次の目標を選ぶ */
					if (distToRoamTarget <= m_stopDistance)
					{
						PickNewRoamTarget();
					}

					BuildInputToTarget(m_roamTarget);
					return;
				}

				/** 隊列参加中：通常の追従入力 */
				BuildInput();
				return;
			}

			/** 待機命令のとき */

			/** 隊列から離脱する */
			if (m_isFollowing)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 親が一定距離以上離れたら徘徊を開始する */
			if (!manager->IsRoaming(m_owner) && distDaddy >= m_roamTriggerDistance)
			{
				manager->RegisterRoaming(m_owner);
				PickNewRoamTarget();
			}

			/** 徘徊中 */
			if (manager->IsRoaming(m_owner))
			{
				const float distToRoamTarget = GetDistanceToTarget(m_roamTarget);

				/** 目標地点に到達したら次の目標を選ぶ */
				if (distToRoamTarget <= m_stopDistance)
				{
					PickNewRoamTarget();
				}

				BuildInputToTarget(m_roamTarget);
				return;
			}

			/** 待機命令中かつ親が近い場合はその場で待機する */
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}


		void NaughtyChildPenguinAI::PickNewRoamTarget()
		{
			std::uniform_real_distribution<float> dist(-m_roamRadius, m_roamRadius);
			auto& engine = GetRandomEngine();

			/** 円内のランダムな座標を選ぶ（拒絶サンプリング） */
			const Vector3& currentPos = m_owner->GetTransform().m_position;
			for (int i = 0; i < 10; i++)
			{
				const float x = dist(engine);
				const float z = dist(engine);
				if ((x * x + z * z) <= (m_roamRadius * m_roamRadius))
				{
					m_roamTarget = Vector3(currentPos.x + x, currentPos.y, currentPos.z + z);
					return;
				}
			}

			/** 最大試行回数を超えた場合は現在地をそのまま目標にする */
			m_roamTarget = currentPos;
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ClumsyChildPenguinAI（おっちょこちょいペンギン）
		//--------------------------------------------------------------

		ClumsyChildPenguinAI::ClumsyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Clumsy)
			, m_clumsyStateMachine(static_cast<ClumsyChildPenguinStateMachine*>(owner->GetStateMachine()))
		{
			const auto& td = GetTypeData(EnChildPenguinType::Clumsy);
			m_tripChancePerSec = td.tripChancePerSec;
			m_slipChance = td.slipChance;
		}


		void ClumsyChildPenguinAI::Update()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** Managerに登録されている転倒・スリップ中フラグで判定する */
			if (manager->IsDowning(m_owner))
			{
				/** 転倒・スリップ中は移動入力をゼロにして固有ステートの評価を妨げないようにする */
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			/** 待機命令のとき */
			if (!isFollowCmd)
			{
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			const float distDaddy = GetDistanceToDaddy();

			if (!m_isFollowing && distDaddy <= m_joinDistance)
			{
				manager->AddFollower(m_owner);
				m_isFollowing = true;
			}
			else if (m_isFollowing && distDaddy > m_giveUpDistance)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			if (!m_isFollowing)
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			/** スライド解除検出：前フレームがスライド中（PenguinSlidingStateのみ）で */
			/** 今フレームがスライド中でなく、かつ泳ぎ中でもなければスリップ判定を行う */
			const bool isCurrentlySliding = m_stateMachine->IsEqualCurrentState(PenguinSlidingState::ID());
			const bool isCurrentlySwimming = m_stateMachine->IsEqualCurrentState(PenguinSwimmingState::ID());

			if (m_wasSliding && !isCurrentlySliding && !isCurrentlySwimming)
			{
				if (RollUnit() < m_slipChance)
				{
					m_clumsyStateMachine->SetIsSlipped(true);
					m_wasSliding = false;
					return;
				}
			}
			m_wasSliding = isCurrentlySliding;

			/** 泳ぎ中は転倒判定をしない */
			if (isCurrentlySwimming)
			{
				BuildInput();
				return;
			}

			/** 隊列参加中の通常移動入力 */
			BuildInput();

			/** 歩き・走り中のみ転倒判定を行う（秒あたりの確率をフレーム確率に変換） */
			const bool isWalking = m_stateMachine->IsEqualCurrentState(PenguinSneakState::ID());
			const bool isRunning = m_stateMachine->IsEqualCurrentState(PenguinRunState::ID());

			if (isWalking || isRunning)
			{
				const float tripChancePerFrame = m_tripChancePerSec * g_gameTime->GetFrameDeltaTime();
				if (RollUnit() < tripChancePerFrame)
				{
					m_clumsyStateMachine->SetIsTripped(true);
				}
			}
		}


		void ClumsyChildPenguinAI::HelpedByCaringPenguin()
		{
			m_clumsyStateMachine->SetIsHelped(true);
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// CaringChildPenguinAI（世話焼きペンギン）
		//--------------------------------------------------------------

		CaringChildPenguinAI::CaringChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Caring)
		{
			const auto& td = GetTypeData(EnChildPenguinType::Caring);
			m_interventionRange = td.interventionRange;
		}


		void CaringChildPenguinAI::Update()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 追従命令のとき */
			if (isFollowCmd)
			{
				/** 担当対象が消えていたら（死亡など）クリアする */
				if (m_interventionTarget != nullptr)
				{
					const auto& childList = manager->GetChildPenguin();
					const bool exists = std::find(childList.begin(), childList.end(), m_interventionTarget) != childList.end();
					if (!exists)
					{
						manager->UnregisterAssigned(m_interventionTarget);
						m_interventionTarget = nullptr;
					}
				}

				/** 助け終わったらターゲットをクリアして次を探す */
				if (m_interventionTarget != nullptr &&
					m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Clumsy)
				{
					if (!manager->IsDowning(m_interventionTarget))
					{
						/** 起き上がり完了 → 介入終了 */
						ReleaseSuppression(m_interventionTarget);
						manager->UnregisterAssigned(m_interventionTarget);
						m_interventionTarget = nullptr;
					}
				}
				else if (m_interventionTarget != nullptr)
				{
					/** 制止対象の命令が Follow になったら制止を解除する */
					ReleaseSuppression(m_interventionTarget);
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}

				/** 担当がいなければ新たに探す */
				if (m_interventionTarget == nullptr)
				{
					const auto& assigned = manager->GetAssignedTargets();
					const Vector3& myPos = m_owner->GetTransform().m_position;

					ChildPenguin* target = manager->FindNearestDowning(myPos, assigned, m_interventionRange);
					if (target != nullptr)
					{
						m_interventionTarget = target;
						manager->RegisterAssigned(m_interventionTarget);
					}
				}

				/** 担当のおっちょこちょいがいる場合 */
				if (m_interventionTarget != nullptr)
				{
					/** 助けに向かう間は隊列から外れる */
					if (m_isFollowing)
					{
						manager->RemoveFollower(m_owner);
						m_isFollowing = false;
					}

					if (IsCloseEnoughTo(m_interventionTarget))
					{
						/** 十分近づいたら介入処理を適用してその場で待機する */
						ApplyIntervention(m_interventionTarget);
						m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
					}
					else
					{
						/** ターゲットの座標へ向かって移動する */
						BuildInputToTarget(m_interventionTarget->GetTransform().m_position);
					}
					return;
				}

				/** 介入対象がいなければ通常の追従行動 */
				const float distDaddy = GetDistanceToDaddy();

				if (!m_isFollowing && distDaddy <= m_joinDistance)
				{
					manager->AddFollower(m_owner);
					m_isFollowing = true;
				}
				else if (m_isFollowing && distDaddy > m_giveUpDistance)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}

				if (!m_isFollowing)
				{
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
					return;
				}

				BuildInput();
				return;
			}

			/** 待機命令のとき */
			if (m_isFollowing)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** おっちょこちょいへの介入：助け終わったらターゲットをクリアして次を探す */
			if (m_interventionTarget != nullptr &&
				m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Clumsy)
			{
				if (!manager->IsDowning(m_interventionTarget))
				{
					/** 起き上がり完了 → 介入終了 */
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}
			}

			/** 担当対象が消えていたら（死亡など）クリアする */
			if (m_interventionTarget != nullptr)
			{
				const auto& childList = manager->GetChildPenguin();
				const bool exists = std::find(childList.begin(), childList.end(), m_interventionTarget) != childList.end();
				if (!exists)
				{
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}
			}

			/** 担当がいなければ新たに探す */
			if (m_interventionTarget == nullptr)
			{
				const auto& assigned = manager->GetAssignedTargets();
				const Vector3& myPos = m_owner->GetTransform().m_position;

				/** 優先①：倒れているおっちょこちょい */
				ChildPenguin* target = manager->FindNearestDowning(myPos, assigned, m_interventionRange);

				/** 優先②：問題行動中の甘えん坊・やんちゃ */
				if (target == nullptr)
				{
					target = manager->FindNearestNeedingSupervision(myPos, assigned, m_interventionRange);
				}

				if (target != nullptr)
				{
					m_interventionTarget = target;
					manager->RegisterAssigned(m_interventionTarget);
				}
			}

			/** 担当対象がいる場合 */
			if (m_interventionTarget != nullptr)
			{
				if (IsCloseEnoughTo(m_interventionTarget))
				{
					/** 十分近づいたら介入処理を適用してその場で待機する */
					ApplyIntervention(m_interventionTarget);
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				}
				else
				{
					/** ターゲットの座標へ向かって移動する */
					BuildInputToTarget(m_interventionTarget->GetTransform().m_position);
				}
				return;
			}

			/** 介入対象がいなければその場で待機する */
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}


		bool CaringChildPenguinAI::IsCloseEnoughTo(const ChildPenguin* target) const
		{
			return GetDistanceToTarget(target->GetTransform().m_position) <= INTERVENTION_REACH_DISTANCE;
		}


		void CaringChildPenguinAI::ApplyIntervention(ChildPenguin* target) const
		{
			switch (target->GetChildPenguinType())
			{
			case EnChildPenguinType::Clumsy:
			{
				/** おっちょこちょいを助けて即座に起き上がらせる */
				auto* ai = static_cast<ClumsyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->HelpedByCaringPenguin();
				}
				break;
			}
			case EnChildPenguinType::Clingy:
			{
				/** 甘えん坊を制止する */
				auto* ai = static_cast<ClingyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(true);
				}
				break;
			}
			case EnChildPenguinType::Naughty:
			{
				/** やんちゃを制止する */
				auto* ai = static_cast<NaughtyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(true);
				}
				break;
			}
			default:
				break;
			}
		}


		void CaringChildPenguinAI::ReleaseSuppression(ChildPenguin* target) const
		{
			switch (target->GetChildPenguinType())
			{
			case EnChildPenguinType::Clingy:
			{
				/** 甘えん坊の制止を解除する */
				auto* ai = static_cast<ClingyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(false);
				}
				break;
			}
			case EnChildPenguinType::Naughty:
			{
				/** やんちゃの制止を解除する */
				auto* ai = static_cast<NaughtyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(false);
				}
				break;
			}
			default:
				break;
			}
		}
	}
}
