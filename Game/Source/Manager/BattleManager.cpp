/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
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
		constexpr float SUB_CAMERA_HEIGHT = 80.0f;
	}


	BattleManager* BattleManager::m_instance = nullptr;


	void BattleManager::Update()
	{
		/** バトルの状態を確認 */
		m_battleState = CheckBattleState();

		/** ゲーム終了なら更新処理をブロック */
		if (m_battleState != EnBattleState::Playing) return;


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
		for (auto* enemy : enemies)
		{
			if (enemy == nullptr) continue;
			if (!enemy->GetEnemyStateMachine()->IsAttack()) continue;

			isAnyAttacking = true;
			break;
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

		/** 攻撃対象の子ペンギンが特定できなければ終了 */
		if (targetChild == nullptr) return;

		/** ターゲット座標・カメラ座標をセットする */
		const Vector3 targetPos = targetChild->GetTransform().m_position;

		/** 親ペンギン→子ペンギンの方向にカメラを配置する */
		Vector3 toTarget = targetPos - daddyPos;
		toTarget.y = 0.0f;
		toTarget.Normalize();

		const Vector3 cameraPos = Vector3(
			targetPos.x - toTarget.x * SUB_CAMERA_DISTANCE,
			targetPos.y + SUB_CAMERA_HEIGHT,
			targetPos.z - toTarget.z * SUB_CAMERA_DISTANCE
		);

		nsBeastEngine::SubCameraManager::Get().SetTargetPosition(targetPos);
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