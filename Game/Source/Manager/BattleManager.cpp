/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "Graphics/Camera/SubCameraManager.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStatus.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Manager/FeverTimeManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"


namespace app
{
	namespace
	{
		/** サブカメラのターゲットからの距離 */
		constexpr float SUB_CAMERA_DISTANCE = 150.0f;
		/** サブカメラのターゲットからの高さ */
		constexpr float SUB_CAMERA_HEIGHT = 60.0f;
		/** ペンギンのルート座標から注視点までのYオフセット */
		constexpr float SUB_CAMERA_TARGET_HEIGHT = 0.0f;

		/** スニークが有効になるシロクマへの最大距離。
		 *  走って近づくとクマは350前後で起きるため、それより十分手前から
		 *  忍び足を始められる距離にする（スニークヒントの表示距離とも揃える） */
		constexpr float SNEAK_AVAILABLE_DIST = 600.0f;
		constexpr float SNEAK_AVAILABLE_DIST_SQ = SNEAK_AVAILABLE_DIST * SNEAK_AVAILABLE_DIST;
	}


	BattleManager* BattleManager::m_instance = nullptr;


	void BattleManager::Update()
	{
		/** バトルの状態を確認 */
		m_battleState = CheckBattleState();

		/** ゲーム終了なら更新処理をブロック。サブカメラが出ていれば閉じる */
		if (m_battleState != EnBattleState::Playing)
		{
			if (m_isSubCameraActive)
			{
				nsBeastEngine::SubCameraManager::Get().End();
				m_isSubCameraActive = false;
				m_lastTargetChild = nullptr;
			}
			return;
		}


		//--------------------------------------------//
		// タイムの通知
		//--------------------------------------------//
		if (m_onTimeChanged)
		{
			m_onTimeChanged(m_currentTime);
		}


		//--------------------------------------------//
		// 残り子ペンギン数の通知
		//--------------------------------------------//
		if (m_onRescuedNumChanged)
		{
			const int rescued = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
			const int total = ScoreManager::GetInstance().GetTotalCount();
			m_onRescuedNumChanged(rescued, total);
		}



		//--------------------------------------------//
		// クマのリアクション通知
		//--------------------------------------------//
		if (m_onBearReactionChanged) m_onBearReactionChanged();


		//--------------------------------------------//
		// 睡眠中クマの通知
		// 探索・UIセットはlambda内で完結する
		//--------------------------------------------//
		if (m_onSleepingEnemyChanged)
		{
			m_onSleepingEnemyChanged();
		}


		//--------------------------------------------//
		// ミニマップの更新
		//--------------------------------------------//
		if (m_onMiniMapChanged)
		{
			ui::ActorPositions positions;


			// 子ペンギンの座標取得
			const auto& childs = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();

			for (const auto* child : childs)
			{
				switch (child->GetChildPenguinType())
				{
				case actor::EnChildPenguinType::Serious:
					positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Serious)).push_back(child->GetTransform().m_position);
					break;
				case actor::EnChildPenguinType::Clingy:
					positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Clingy)).push_back(child->GetTransform().m_position);
					break;
				case actor::EnChildPenguinType::Naughty:
					positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Naughty)).push_back(child->GetTransform().m_position);
					break;
				case actor::EnChildPenguinType::Clumsy:
					positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Clumsy)).push_back(child->GetTransform().m_position);
					break;
				case actor::EnChildPenguinType::Caring:
					positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Caring)).push_back(child->GetTransform().m_position);
					break;
				default:
					break;
				}
			}

			// ステージオブジェクトの座標を取得する
			auto GetObjectPosition = [&](const std::string& name, ui::EnMiniMapIconType type)
				{
					auto* system = actor::StageSystem::GetInstance();

					// 座標を取得する
					Vector3 objectPosition = Vector3::One;

					// 座標の取得に失敗するとVector3::Zeroが返ってくる
					// Vector3::Zeroが帰ってきたら終了する
					int index = 1;
					while (true)
					{
						const std::string keyName = name + std::to_string(index);

						objectPosition = system->GetObjectPosition(keyName);

						if (objectPosition.IsEqual(Vector3::Zero)) break;

						positions.at(static_cast<uint8_t>(type)).push_back(objectPosition);
						index++;
					}
				};


			GetObjectPosition("bearHome_", ui::EnMiniMapIconType::BearNest);
			GetObjectPosition("igloo_", ui::EnMiniMapIconType::Igloo);


			// シロクマの座標を取得する
			const auto& enemies = actor::EnemyManager::GetInstance()->GetEnemies();

			for (const auto* enemy : enemies)
			{
				K2_ASSERT(enemy, "エネミーがnullptr");
				positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Bear)).push_back(enemy->GetTransform().m_position);
			}


			// 渦潮の座標を取得する
			// 配置インデックスをそのまま添字にした固定長配列として渡すことで、
			// 他の渦潮の生成・削除によって既存の渦潮の配列位置がずれないようにする
			// （ミニマップ側は配列位置だけでアイコンと渦潮を対応付けているため、
			//   ずれると既存アイコンが別の渦潮の座標を指してしまい点滅して見える）。
			// 未使用の添字はミニマップの表示範囲外になる座標で埋めておき、非表示のままにする。
			if (auto* wm = nature::WhirlpoolManager::GetInstance())
			{
				const Vector3 OUT_OF_MAP_RANGE_POS(1.0e8f, 0.0f, 1.0e8f);

				auto& whirlpoolPositions = positions.at(static_cast<uint8_t>(ui::EnMiniMapIconType::Whirlpool));
				whirlpoolPositions.assign(wm->GetWhirlpoolCountMax(), OUT_OF_MAP_RANGE_POS);

				wm->ForEach([&](nature::Whirlpool* whirlpool)
					{
						const uint8_t index = whirlpool->GetIndex();
						if (index < whirlpoolPositions.size())
						{
							whirlpoolPositions[index] = whirlpool->GetTransform().m_position;
						}
					});
			}


			m_onMiniMapChanged(positions);
		}



		//--------------------------------------------//
		// 渦潮の警告座標の通知
		//--------------------------------------------//
		if (m_wpWarningChanged)
		{
			std::vector<Vector3> warningPositions;
			nature::WhirlpoolManager::GetInstance()->ForEach([&](nature::Whirlpool* whirlpool)
				{
					warningPositions.push_back(whirlpool->GetTransform().m_position);
				});
			m_wpWarningChanged(warningPositions);
		}



		//--------------------------------------------//
		// スピードアップ中の通知
		//--------------------------------------------//
		if (m_speedLineChanged)
		{
			const auto* cm = actor::ChildPenguinManager::GetInstance();
			const bool isUlt = cm->IsUltActive();
			const bool isTriangle = cm->GetCurrentFormationType() == actor::EnFormationType::Triangle;
			const bool isCircle = cm->GetCurrentFormationType() == actor::EnFormationType::Circle;

			const auto* dsm = m_daddyPenguin->GetStateMachine();
			const bool isDash = dsm->GetIsDash();
			const bool isSwim = dsm->IsSwimming();
			const bool isMove = isDash || isSwim;

			m_speedLineChanged(isUlt && (isTriangle || isCircle) && isMove);
		}



		//--------------------------------------------//
		// スニーク可否の更新
		//--------------------------------------------//
		UpdateSneakAvailability();

		//--------------------------------------------//
		// サブカメラの更新
		//--------------------------------------------//
		UpdateSubCamera();
	}


	void BattleManager::UpdateSneakAvailability()
	{
		const auto enemies = actor::EnemyManager::GetInstance()->GetEnemies();
		const Vector3 daddyPos = actor::ChildPenguinManager::GetInstance()->GetDaddyPosition();

		float nearestDistSq = FLT_MAX;
		for (const auto* enemy : enemies)
		{
			if (enemy == nullptr) continue;
			Vector3 diff = enemy->GetTransform().m_position - daddyPos;
			diff.y = 0.0f; // 高低差は無視
			const float distSq = diff.LengthSq();
			if (distSq < nearestDistSq) nearestDistSq = distSq;
		}

		m_isSneakAvailable = (nearestDistSq <= SNEAK_AVAILABLE_DIST_SQ);
	}


	void BattleManager::UpdateSubCamera()
	{
		const auto enemies = actor::EnemyManager::GetInstance()->GetEnemies();
		const auto controllers = actor::EnemyManager::GetInstance()->GetControllers();
		const int enemyCount = static_cast<int>(enemies.size());
		const Vector3 daddyPos = actor::ChildPenguinManager::GetInstance()->GetDaddyPosition();

		/**
		 * チェイス状態（攻撃前）のシロクマからターゲットをキャッシュする。
		 * GetFoundPenguin()はEnterAttack()でクリアされるため、
		 * 攻撃フェーズに入る前のチェイス中に毎フレーム更新しておく必要がある。
		 * IsAttack()を条件にしないことで、チェイス中にも確実にキャッシュできる。
		 */
		{
			float nearestDistSq = FLT_MAX;
			for (int i = 0; i < enemyCount; i++)
			{
				auto* enemy = enemies[i];
				if (enemy == nullptr) continue;
				auto* controller = controllers[i];
				if (controller == nullptr) continue;
				const auto* child = controller->GetFoundPenguin();
				if (child == nullptr) continue;
				Vector3 diff = child->GetTransform().m_position - daddyPos;
				diff.y = 0.0f;
				const float distSq = diff.LengthSq();
				if (distSq < nearestDistSq)
				{
					nearestDistSq = distSq;
					m_lastTargetChild = child;
				}
			}
		}

		/** チェイス中またはアタック中のシロクマを探す */
		bool isAnyActive = false;
		const actor::Enemy* attackingEnemy = nullptr;
		const actor::Enemy* chasingEnemy = nullptr;
		for (auto* enemy : enemies)
		{
			if (enemy == nullptr) continue;
			const auto* sm = enemy->GetEnemyStateMachine();
			if (sm->IsAttack())
			{
				isAnyActive = true;
				if (attackingEnemy == nullptr) attackingEnemy = enemy;
			}
			else if (sm->IsChasing())
			{
				isAnyActive = true;
				if (chasingEnemy == nullptr) chasingEnemy = enemy;
			}
		}

		/** チェイスもアタックもしていなければサブカメラを停止する */
		if (!isAnyActive)
		{
			if (m_isSubCameraActive)
			{
				nsBeastEngine::SubCameraManager::Get().End();
				m_isSubCameraActive = false;
				m_lastTargetChild = nullptr;
			}
			return;
		}

		/** チェイスまたはアタック中のシロクマがいればサブカメラを起動する */
		if (!m_isSubCameraActive)
		{
			nsBeastEngine::SubCameraManager::Get().Begin();
			m_isSubCameraActive = true;
		}

		/** チェイス中にキャッシュしたターゲットを使用してカメラを追従する */
		const actor::ChildPenguin* targetChild = m_lastTargetChild;

		/** ダングリングポインタのガード：遅延deleteにより解放済みの場合はリストに存在しない */
		if (targetChild != nullptr)
		{
			const auto& list = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();
			bool stillExists = false;
			for (const auto* p : list)
			{
				if (p == targetChild) { stillExists = true; break; }
			}
			if (!stillExists || targetChild->GetStatus<actor::ChildPenguinStatus>()->IsDead())
			{
				m_lastTargetChild = nullptr;
				targetChild = nullptr;
			}
		}

		/** 攻撃対象の子ペンギンが特定できなければ終了 */
		if (targetChild == nullptr) return;

		/** ターゲット座標・カメラ座標をセットする */
		const Vector3 basePos = targetChild->GetTransform().m_position;
		const Vector3 targetPos = basePos + Vector3(0.0f, SUB_CAMERA_TARGET_HEIGHT, 0.0f);

		// カメラ方向: 子ペンギン→シロクマ方向にカメラを向ける。
		// カメラはペンギンの「シロクマと反対側」に置き、ペンギン越しにシロクマが迫る画を映す。
		// bearToPenguin = penguinPos - bearPos（シロクマ→ペンギン方向）
		// 攻撃中シロクマを優先し、いなければチェイス中シロクマを使う。
		const actor::Enemy* relevantEnemy = (attackingEnemy != nullptr) ? attackingEnemy : chasingEnemy;
		Vector3 bearToPenguin;
		if (relevantEnemy != nullptr)
		{
			bearToPenguin = basePos - relevantEnemy->GetTransform().m_position;
		}
		else
		{
			bearToPenguin = basePos - daddyPos;
		}
		bearToPenguin.y = 0.0f;
		const float bearToPenguinLen = bearToPenguin.Length();
		if (bearToPenguinLen < 0.01f)
		{
			bearToPenguin = Vector3(0.0f, 0.0f, 1.0f);
		}
		else
		{
			bearToPenguin /= bearToPenguinLen;
		}

		// + bearToPenguin でペンギンのシロクマ反対側にカメラを配置する
		const Vector3 cameraPos = Vector3(
			targetPos.x + bearToPenguin.x * SUB_CAMERA_DISTANCE,
			targetPos.y + SUB_CAMERA_HEIGHT,
			targetPos.z + bearToPenguin.z * SUB_CAMERA_DISTANCE
		);

		// 注視点: シロクマを中心に映す。シロクマがいない場合のみペンギンにフォールバック。
		const Vector3 lookAtPos = (attackingEnemy != nullptr)
			? attackingEnemy->GetTransform().m_position + Vector3(0.0f, SUB_CAMERA_TARGET_HEIGHT, 0.0f)
			: targetPos;

		nsBeastEngine::SubCameraManager::Get().SetTargetPosition(lookAtPos);
		nsBeastEngine::SubCameraManager::Get().SetCameraPosition(cameraPos);
	}


	void BattleManager::ResetObservers()
	{
		m_onTimeChanged = nullptr;
		m_onRescuedNumChanged = nullptr;
		m_onSleepingEnemyChanged = nullptr;
		m_onBearReactionChanged = nullptr;
		m_onMiniMapChanged = nullptr;
		m_wpWarningChanged = nullptr;
		m_onCPReactionChanged = nullptr;
		m_onEnemyReactionChanged = nullptr;
		m_onReactionTargetDestroyed = nullptr;
		m_onImpact = nullptr;
		m_onFormationLevelUp = nullptr;
	}




	//============================================//
	// ゲームの状態遷移処理
	//============================================//

	BattleManager::EnBattleState BattleManager::CheckBattleState() const
	{
		const int collected = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
		const int total = ScoreManager::GetInstance().GetTotalCount();
		const bool isTimeUp = TimeManager::GetInstance().IsTimeUp();

		auto* feverManager = FeverTimeManager::GetInstance();

		/** ステージ上の全員を救助した瞬間、まだフィーバーが発生していなければ、
		 *  終了とせず即座にフィーバータイムへ入る（feverEnabledがfalseのステージでは何もしない） */
		if (collected == total && !feverManager->HasPendingDrops())
		{
			feverManager->TryStartFeverOnAllCaught();
		}

		/** フィーバー中、まだ投下待ちの子ペンギンがいる間は総数がこれから増える予定なので、
		 *  瞬間的に collected == total になっても全員救助扱いにしない */
		const bool feverPending = feverManager->HasPendingDrops();

		/**
		 *	[終了条件]
		 *	1. 全員救助（救助数 == ステージ上の総数、ただしフィーバーの投下待ちが無い場合のみ）
		 *	2. タイムアップ
		 */
		if ((collected == total && !feverPending) || isTimeUp)
		{
			return EnBattleState::Finished;
		}

		/** どちらも満たしていなければ継続 */
		return EnBattleState::Playing;
	}
}
