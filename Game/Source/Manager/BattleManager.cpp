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


	namespace
	{
		constexpr int CLEAR_COUNT = 50;	/** クリア条件のペンギンの数 */
	}


	void BattleManager::Update()
	{
		/** バトルの状態を確認 */
		m_battleState = CheckBattleState();

		/** ゲームクリアorゲームオーバーなら更新処理をブロック */
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
		/** スコアマネージャーから、今のスコアと最大スコアを取得 */
		auto& score = ScoreManager::GetInstance();
		const int collected = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
		const int total = score.GetTotalCount();
		/** タイムマネージャーから、タイムアップしているかどうかを取得 */
		const bool isTimeUp = TimeManager::GetInstance().IsTimeUp();

		/**
		 *	【クリア条件】
		 *	1. 一定数以上救助 & 全ての子ペンギンを救助
		 *	2. 一定数以上救助 & タイムアップ
		 */
		if ((collected >= CLEAR_COUNT && collected == total)
			|| (collected >= CLEAR_COUNT && isTimeUp))
		{
			return EnBattleState::Clear;
		}

		/**
		 *	【ゲームオーバー条件】
		 *	1. ステージ上の子ペンギンが一定数未満（シロクマに食べられた）
		 *	2. 救助数が一定以下 & タイムアップ
		 */
		if ((total < CLEAR_COUNT)
			|| (collected < CLEAR_COUNT && isTimeUp))
		{
			return EnBattleState::GameOver;
		}

		/** どちらも満たしていなければ継続 */
		return EnBattleState::Playing;
	}
}