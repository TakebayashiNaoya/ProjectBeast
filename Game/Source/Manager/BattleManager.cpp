/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStatus.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Graphics/Camera/SubCameraManager.h"


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
		// 睡眠中クマの通知
		// 探索・UIセットはlambda内で完結する
		//--------------------------------------------//
		if (m_onSleepingEnemyChanged)
		{
			m_onSleepingEnemyChanged();
		}


		//--------------------------------------------//
		// サブカメラの更新
		//--------------------------------------------//
		UpdateSubCamera();
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

		/** 攻撃中のシロクマを探す */
		bool isAnyAttacking = false;
		const actor::Enemy* attackingEnemy = nullptr;
		for (auto* enemy : enemies)
		{
			if (enemy == nullptr) continue;
			if (!enemy->GetEnemyStateMachine()->IsAttack()) continue;

			isAnyAttacking = true;
			if (attackingEnemy == nullptr) attackingEnemy = enemy;
		}

		/** 攻撃中のシロクマがいなければサブカメラを停止する */
		if (!isAnyAttacking)
		{
			if (m_isSubCameraActive)
			{
				nsBeastEngine::SubCameraManager::Get().End();
				m_isSubCameraActive = false;
				m_lastTargetChild = nullptr;
			}
			return;
		}

		/** 攻撃中のシロクマがいればサブカメラを起動する */
		if (!m_isSubCameraActive)
		{
			nsBeastEngine::SubCameraManager::Get().Begin();
			m_isSubCameraActive = true;
		}

		/** チェイス中にキャッシュしたターゲットを使用してカメラを追従する */
		const actor::ChildPenguin* targetChild = m_lastTargetChild;

		/** ターゲットが死亡していたらキャッシュをクリアしてサブビューを非表示にする */
		if (targetChild != nullptr && targetChild->GetStatus<actor::ChildPenguinStatus>()->IsDead())
		{
			m_lastTargetChild = nullptr;
			targetChild = nullptr;
		}

		/** 攻撃対象の子ペンギンが特定できなければ終了 */
		if (targetChild == nullptr) return;

		/** ターゲット座標・カメラ座標をセットする */
		const Vector3 basePos   = targetChild->GetTransform().m_position;
		const Vector3 targetPos = basePos + Vector3(0.0f, SUB_CAMERA_TARGET_HEIGHT, 0.0f);

		// カメラ方向: 子ペンギン→シロクマ方向にカメラを向ける。
		// カメラはペンギンの「シロクマと反対側」に置き、ペンギン越しにシロクマが迫る画を映す。
		// bearToPenguin = penguinPos - bearPos（シロクマ→ペンギン方向）
		Vector3 bearToPenguin;
		if (attackingEnemy != nullptr)
		{
			bearToPenguin = basePos - attackingEnemy->GetTransform().m_position;
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
	}




	//============================================//
	// ゲームの状態遷移処理
	//============================================//

	BattleManager::EnBattleState BattleManager::CheckBattleState() const
	{
		const int collected = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
		const int total = ScoreManager::GetInstance().GetTotalCount();
		const bool isTimeUp = TimeManager::GetInstance().IsTimeUp();

		/**
		 *	[終了条件]
		 *	1. 全員救助（救助数 == ステージ上の総数）
		 *	2. タイムアップ
		 */
		if (collected == total || isTimeUp)
		{
			return EnBattleState::Finished;
		}

		/** どちらも満たしていなければ継続 */
		return EnBattleState::Playing;
	}
}