/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Orca/Orca.h"
#include "Source/Actor/Character/Orca/OrcaManager.h"
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
	BattleManager* BattleManager::m_instance = nullptr;


	void BattleManager::Update()
	{
		/** バトルの状態を確認 */
		m_battleState = CheckBattleState();

		/** ゲーム終了なら更新処理をブロック */
		if (m_battleState != EnBattleState::Playing)
		{
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
			const bool isTriangle = cm->GetCurrentFormationType() == actor::EnFormationType::Triangle;

			const auto* dsm = m_daddyPenguin->GetStateMachine();
			const bool isDash = dsm->GetIsDash();
			const bool isSwim = dsm->IsSwimming();
			const bool isMove = isDash || isSwim;

			// 三角陣のパッシブ（スピード特化）で動いている間だけ弱めに出す
			constexpr float SPEED_LINE_STRENGTH_TRIANGLE_PASSIVE = 0.5f;
			const float strength = (isMove && isTriangle) ? SPEED_LINE_STRENGTH_TRIANGLE_PASSIVE : 0.0f;
			m_speedLineChanged(strength);
		}
	}


	void BattleManager::ResetObservers()
	{
		m_onTimeChanged = nullptr;
		m_onRescuedNumChanged = nullptr;
		m_onSleepingEnemyChanged = nullptr;
		m_onBearReactionChanged = nullptr;
		m_wpWarningChanged = nullptr;
		m_onCPReactionChanged = nullptr;
		m_onEnemyReactionChanged = nullptr;
		m_onOrcaReactionChanged = nullptr;
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
