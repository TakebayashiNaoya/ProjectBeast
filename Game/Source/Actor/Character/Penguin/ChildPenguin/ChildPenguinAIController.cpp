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
			/** 甘えん坊ペンギンのエフェクトオフセット */
			const Vector3 CLINGY_HART_EFFECT_POSITION = { 0.0f,30.0f,0.0f };
			/** 甘えん坊ペンギンのエフェクトスケール */
			const Vector3 CLINGY_HART_EFFECT_SCALE = { 10.0f,10.0f,10.0f };
			
			/**
			 * @brief ヒステリシス幅
			 * @details フェーズを下げるとき、閾値からさらにこの距離だけ内側に入って初めて下げる。
			 *          m_stopDistance より小さい値にすること。
			 */
			constexpr float HYSTERESIS = 5.0f;

			/**
			 * @brief フェーズを上げるためのマージン（遊び）
			 * @details 閾値からさらにこの距離だけ外側に離れて初めてフェーズを上げる。
			 */
			constexpr float PHASE_UP_MARGIN = 5.0f;

			/**
			 * @brief 停止判定で共通利用する速度の閾値（速度の二乗で比較）
			 * @details Walk → Stop 遷移と BuildInputToTarget() 冒頭の強制 Stop 判定の
			 *          両方でこの定数を参照する。
			 *          lerpの慣性が残っているうちは Stop に入らず Walk を維持し、
			 *          停止アニメ中も滑り続ける問題を防ぐ。
			 */
			constexpr float STOP_VELOCITY_THRESHOLD_SQ = 1.0f;

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
			, m_hartEffectHandle(INVALID_EFFECT_HANDLE)
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
			if (direction.LengthSq() < FLT_EPSILON)
			{
				return Vector3::Zero;  // 呼び出し側の LengthSq チェックで弾かれる
			}
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
			if (direction.LengthSq() < FLT_EPSILON)
			{
				return Vector3::Zero;
			}
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
			// SlideEnd アニメーション中は入力をゼロにして自然に滑り止まるのを待つ
			if (m_stateMachine->IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

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
			 * 上げる : 設定された距離（m_walkDistance, m_runDistance）を超えたらすぐに上げる
			 * 下げる : 一度 Run や Slide になったら m_walkDistance まで距離が縮まったら Walk に戻し、
			 *          Walk から m_stopDistance 以内かつ速度がほぼゼロになったら Stop に落とす。
			 *          これにより Slide → Walk → Stop の3段階を確実に踏み、
			 *          lerpの慣性が残ったままオーバーシュートしても Walk に留まって再減速できる。
			 */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
			{
				if (distToTarget > m_runDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Run; }
				else if (distToTarget > m_stopDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Walk; }
				break;
			}

			case MovePhase::Walk:
				if (distToTarget > m_runDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Slide; }
				else if (distToTarget > m_walkDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Run; }
				else if (distToTarget <= m_stopDistance - HYSTERESIS)
				{
					// lerpの慣性が残っている間は Stop に入らず Walk を維持する。
					// 慣性が残ったまま Stop になるとアニメーションが止まっても滑り続けるため。
					//const Vector3& currentVel = m_stateMachine->GetCurrentVelocity();
					//if (currentVel.LengthSq() < STOP_VELOCITY_THRESHOLD_SQ)
					//{
					m_movePhase = MovePhase::Stop;
					//}
				}
				break;

			case MovePhase::Run:
				/** さらに離されたら Slide へ上げる */
				if (distToTarget > m_runDistance + PHASE_UP_MARGIN) { m_movePhase = MovePhase::Slide; }
				/** m_walkDistance 以内に入ったら Walk へ戻し、そこから Stop へ段階的に落とす */
				else if (distToTarget <= m_walkDistance - HYSTERESIS) { m_movePhase = MovePhase::Walk; }
				break;

			case MovePhase::Slide:
				/** m_walkDistance 以内に入ったら Walk へ戻し、そこから Stop へ段階的に落とす */
				if (distToTarget <= m_walkDistance - HYSTERESIS) { m_movePhase = MovePhase::Walk; }
				break;
			}

			/**
			 * 移動方向（常に正規化済みベクトルを渡す）
			 *
			 * Move() は moveDirection * (moveSpeed * speedMultiplier) で目標速度を計算するため、
			 * moveDirection のスケール（長さ）を変えると本来の速度設定が崩れてしまう。
			 * 基本速度の調整は SetMoveSpeed()、目標手前での減速やブレーキは SetSpeedMultiplier() 経由で行う。
			 */
			const Vector3 moveDirection = CalculateDirectionToTarget(targetPos);

			float speedMultiplier = 1.0f;

			// 停止距離が有効な場合のみ、停止距離の2倍以内で倍率を 1.0 から 0.0 へ徐々に下げる
			if (m_stopDistance > 0.0f)
			{
				const float brakeRange = m_stopDistance * 2.0f;
				if (distToTarget > m_stopDistance && distToTarget < brakeRange && m_movePhase != MovePhase::Stop)
				{
					// 停止距離の外側にいる間だけ減速補間し、入力と実移動の不一致を防ぐ
					const float ratio = (distToTarget - m_stopDistance) / (brakeRange - m_stopDistance);
					speedMultiplier = max(0.0f, min(1.0f, ratio));
				}
			}

			// 計算した倍率をステートマシンに渡し、物理処理(Lerp)の目標速度を落とす
			m_stateMachine->SetSpeedMultiplier(speedMultiplier);

			/** フェーズに応じてAIControllerInputを組み立てる */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				/** 停止 */
				// 目標座標の近くで自然に止まるため、向きの調整は不要
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				break;

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

			// 定数化して判定を明確にする
			constexpr float ENTER_DISTANCE = 150.0f;

			// 青い円に十分近づいたらワープ発動
			if (dirToTarget.Length() < ENTER_DISTANCE)
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);

				// 子ペンギンの現在位置を基準に最も近いイグルーの中心座標を取得
				Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
				Vector3 insidePos = iglooPos;

				constexpr float IGLOO_INSIDE_CIRCLE = 360.0f;
				constexpr float IGLOO_INSIDE_RADIUS = 60.0f; // かまくらの中に収まる半径

				// 一列に並ぶのを防ぐため、浮動小数点で円形にばらけさせる
				auto& engine = GetRandomEngine();
				std::uniform_real_distribution<float> angleDist(0.0f, IGLOO_INSIDE_CIRCLE);
				std::uniform_real_distribution<float> radiusDist(0.0f, IGLOO_INSIDE_RADIUS);

				constexpr float IGLOO_INSIDE_HALF_CIRCLE = 180.0f;

				float angleRad = angleDist(engine) * (Math::PI / IGLOO_INSIDE_HALF_CIRCLE);
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


				// =========================================================
				// ★ かまくらマネージャーにこの子ペンギンを格納（登録）する！
				// =========================================================
				IglooManager::GetInstance().AddPenguin(m_owner);

				// マネージャーに入室完了を報告（イベント終了判定用）
				ChildPenguinManager::GetInstance()->FinishEnterIglooOne();
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


		void ChildPenguinAIController::ForceEjectFromIgloo(const Vector3& iglooPos)
		{
			// 1. 各種イベントフラグを強制解除（通常のAI処理に戻すため）
			m_isEnterIglooMode = false;
			m_isInsideIgloo = false;
			m_owner->SetInsideIgloo(false);

			constexpr float EJECT_OFFSET_RANGE = 60.0f; // かまくらの中心から弾き出される範囲（半径）
			constexpr float EJECT_UP_OFFSET = 50.0f; // 弾き出される際の上方向のオフセット

			// 2. 弾き出される座標をランダムに散らす
			// かまくらの中心(iglooPos)を基準に、周囲にランダムに配置します
			auto& engine = GetRandomEngine();
			std::uniform_real_distribution<float> offsetDist(-EJECT_OFFSET_RANGE, EJECT_OFFSET_RANGE);

			Vector3 spawnPos = iglooPos;
			spawnPos.x += offsetDist(engine);
			spawnPos.z += offsetDist(engine);

			// 少し上空から落とすことで、弾き飛ばされた感を演出します
			//spawnPos.y +=EJECT_UP_OFFSET;

			// 3. キャラクターコントローラーとステートマシン両方をワープ
			m_owner->GetCharacterController()->SetPosition(spawnPos);
			m_owner->GetCharacterController()->RequestTeleport(); // ワープを確定
			m_stateMachine->SetPosition(spawnPos);

			// 4. 強制的に空中に放り出された判定にするため、入力をゼロにする
			// （これによって自動的にジャンプ・落下ステート等へ遷移します）
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
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
			, m_clingyEffectHandle(INVALID_EFFECT_HANDLE)
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

			// エフェクトの再生と位置更新を行うラムダ。
			auto updateClingyEffect = [&]()
				{
					// 計測時間。
					m_effectInterval += g_gameTime->GetFrameDeltaTime();
					// ハートエフェクトの生み出される地点。
					const Vector3 hartPos = m_owner->GetTransform().m_position + CLINGY_HART_EFFECT_POSITION;

					// 1秒を超えたらエフェクトを再生。
					if (m_effectInterval > 1.0f)
					{
						const Vector3 hartScl = m_owner->GetTransform().m_scale + CLINGY_HART_EFFECT_SCALE;
						m_clingyEffectHandle = EffectManager::Get().PlayEffect(
							EnEffectKind::ClingyPenguinHart
							, hartPos
							, Quaternion::Identity
							, hartScl
						);
						// インターバルをリセット。
						m_effectInterval = 0.0f;
					}
				};

			// エフェクトを停止するラムダ。
			auto stopEffect = [&]()
				{
					if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
					{
						// エフェクトを停止。
						EffectManager::Get().StopEffect(m_clingyEffectHandle);
						// ハンドルを無効化。
						m_clingyEffectHandle = INVALID_EFFECT_HANDLE;
						// インターバルをリセット。
						m_effectInterval = 0.0f;
					}
				};

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

				if (m_isFollowing)
				{
					updateClingyEffect();
				}

				else if (m_isFollowing && distDaddy > m_giveUpDistance)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;

					stopEffect();
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

			// エフェクト再生。
			if (m_isFollowing && !m_isRestrained)
			{
				updateClingyEffect();
			}

			// エフェクト停止。
			else
			{
				/** 追従から外れた、または制止された場合はエフェクトを即座に停止する */
				if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
				{
					stopEffect();
				}
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
					if (!m_isFollowing)
					{
						manager->RemoveFollower(m_owner);
						m_isFollowing = true;
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