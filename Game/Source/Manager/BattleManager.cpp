/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "BattleManager.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"


namespace app
{
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