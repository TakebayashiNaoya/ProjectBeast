/**
 * @file InGameUIManager.cpp
 * @brief インゲームUIの生成・更新・描画・配線を管理するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "InGameUIManager.h"

#include "Source/Manager/BattleManager.h"
#include "Source/Manager/ScoreManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinController.h"

#include "Source/UI/CountDownMenu.h"
#include "Source/UI/EnemySleepingMenu.h"
#include "Source/UI/FinishMenu.h"
#include"Source/UI/IglooPromptMenu.h"
#include "Source/UI/InGameTimerMenu.h"
#include "Source/UI/Layout.h"
#include "Source/UI/PauseScreenMenu.h"
#include "Source/UI/PBWakingUpTimerMenu.h"
#include "Source/UI/RemainingChildMenu.h"
#include "Source/UI/SearchMenu.h"
#include "Source/UI/SoundOptionMenu.h"
#include "Source/UI/TutorialMenu.h"


namespace app
{
	namespace
	{
		/** 睡眠中クマの探索半径 */
		constexpr float SLEEPING_ENEMY_SEARCH_RANGE = 1000.0f;
	}


	InGameUIManager::InGameUIManager()
	{}


	InGameUIManager::~InGameUIManager()
	{
		/** BattleManagerのfunctionをリセットして、dangling参照を防ぐ */
		BattleManager::GetInstance().ResetObservers();

		/** Layout削除（MenuはLayoutが所有するため個別削除不要） */
		delete m_countDownLayout;
		delete m_timerLayout;
		delete m_finishLayout;
		delete m_remainingChildLayout;
		delete m_pauseLayout;
		delete m_soundOptionLayout;
		delete m_enemySleepingLayout;
		delete m_pbWakingUpTimerLayout;
		delete m_iglooPromptLayout;
		delete m_tutorialLayout;

		for (auto* layout : m_searchLayouts)
		{
			delete layout;
		}
		m_searchLayouts.clear();
	}


	void InGameUIManager::Initialize(actor::DaddyPenguin* daddyPenguin)
	{
		//------------------------------------------------------------
		// Layout・Menuの生成
		//------------------------------------------------------------
		m_countDownLayout = new ui::Layout();
		m_countDownLayout->Initialize<ui::CountDownMenu>(
			"Assets/parameter/countDown/CountDown.json"
		);
		m_countDownMenu = m_countDownLayout->GetMenu<ui::CountDownMenu>();

		m_timerLayout = new ui::Layout();
		m_timerLayout->Initialize<ui::InGameTimerMenu>(
			"Assets/parameter/timer/InGameTimer.json"
		);
		m_timerMenu = m_timerLayout->GetMenu<ui::InGameTimerMenu>();

		m_finishLayout = new ui::Layout();
		m_finishLayout->Initialize<ui::FinishMenu>(
			"Assets/parameter/event/FinishMenu.json"
		);
		m_finishMenu = m_finishLayout->GetMenu<ui::FinishMenu>();

		m_remainingChildLayout = new ui::Layout();
		m_remainingChildLayout->Initialize<ui::RemainingChildMenu>(
			"Assets/parameter/UI/remainingChild/remainingChild.json"
		);
		m_remainingChildMenu = m_remainingChildLayout->GetMenu<ui::RemainingChildMenu>();

		m_pauseLayout = new ui::Layout();
		m_pauseLayout->Initialize<ui::PauseScreenMenu>(
			"Assets/parameter/pause/PauseScreen.json"
		);
		m_pauseMenu = m_pauseLayout->GetMenu<ui::PauseScreenMenu>();

		m_soundOptionLayout = new ui::Layout();
		m_soundOptionLayout->Initialize<ui::SoundOptionMenu>(
			"Assets/parameter/sound/SoundOption.json"
		);
		m_soundOptionMenu = m_soundOptionLayout->GetMenu<ui::SoundOptionMenu>();

		m_enemySleepingLayout = new ui::Layout();
		m_enemySleepingLayout->Initialize<ui::EnemySleepingMenu>(
			"Assets/parameter/UI/enemySleepGauge/sleepGauge.json"
		);
		m_enemySleepingMenu = m_enemySleepingLayout->GetMenu<ui::EnemySleepingMenu>();
		if (m_enemySleepingMenu)
		{
			m_enemySleepingMenu->SetDraw(false);
			m_enemySleepingMenu->SetSleepingRate(0.0f);
		}

		m_pbWakingUpTimerLayout = new ui::Layout();
		m_pbWakingUpTimerLayout->Initialize<ui::PBWakingUpTimerMenu>(
			"Assets/parameter/timer/PBWakingUpTimer.json"
		);
		m_pbWakingUpTimerMenu = m_pbWakingUpTimerLayout->GetMenu<ui::PBWakingUpTimerMenu>();
		if (m_pbWakingUpTimerMenu)
		{
			m_pbWakingUpTimerMenu->SetDraw(false);
		}

		m_iglooPromptLayout = new ui::Layout();
		m_iglooPromptLayout->Initialize<ui::IglooPromptMenu>(
			"Assets/parameter/UI/iglooPrompt/IglooPrompt.json"
		);
		m_iglooPromptMenu = m_iglooPromptLayout->GetMenu<ui::IglooPromptMenu>();
		if (m_iglooPromptMenu)
		{
			m_iglooPromptMenu->SetDraw(false);
		}
		daddyPenguin->GetController()->SetIglooPromptMenu(m_iglooPromptMenu);
    
		m_tutorialLayout = new ui::Layout();
		m_tutorialLayout->Initialize<ui::TutorialMenu>(
			"Assets/parameter/tutorial/Tutorial.json"
		);
		m_tutorialMenu = m_tutorialLayout->GetMenu<ui::TutorialMenu>();

		/** BattleManagerへのUI通知functionを登録 */
		RegisterObservers(daddyPenguin);
	}


	void InGameUIManager::AddSearchLayout(actor::Enemy* enemy)
	{
		auto* layout = new ui::Layout();
		layout->Initialize<ui::SearchMenu>("Assets/parameter/search/Search.json");

		auto* menu = layout->GetMenu<ui::SearchMenu>();
		if (menu)
		{
			menu->SetEnemy(enemy);
			menu->SetIsActive(true);
		}

		m_searchLayouts.push_back(layout);
		m_searchMenus.push_back(menu);
	}


	void InGameUIManager::RegisterObservers(actor::DaddyPenguin* daddyPenguin)
	{
		auto& bm = BattleManager::GetInstance();

		//--------------------------------------------//
		// タイマーUI通知
		//--------------------------------------------//
		bm.SetOnTimeChanged(
			[this](float time)
			{
				if (m_timerMenu)
				{
					m_timerMenu->SetTime(time);
				}
			}
		);

		//--------------------------------------------//
		// 残り子ペンギン数UI通知
		//--------------------------------------------//
		bm.SetOnRescuedNumChanged(
			[this](int rescued, int total)
			{
				if (m_remainingChildMenu)
				{
					m_remainingChildMenu->SetChildNum(rescued);
					m_remainingChildMenu->SetTotalNum(total);
				}
			}
		);

		//--------------------------------------------//
		// 睡眠中クマUI通知
		// daddyPenguinをキャプチャしてlambda内で探索する
		//--------------------------------------------//
		bm.SetOnSleepingEnemyChanged(
			[this, daddyPenguin]()
			{
				/** プレイヤー座標を基準に最近傍の睡眠中クマを探す */
				const Vector3 playerPos = daddyPenguin->GetTransform().m_position;
				auto* enemy = actor::EnemyManager::GetInstance()->GetNearestSleepingEnemy(
					playerPos,
					SLEEPING_ENEMY_SEARCH_RANGE
				);

				const bool isFound = (enemy != nullptr);

				/** 起床ゲージUIへ通知 */
				if (m_enemySleepingMenu)
				{
					if (isFound)
					{
						auto* sm = enemy->GetEnemyStateMachine();
						/** 起床ゲージ（満タン=100）を0〜1に正規化して渡す */
						m_enemySleepingMenu->SetSleepingRate(sm->GetWakeUpGauge() / 100.0f);
						m_enemySleepingMenu->SetTargetPosition(enemy->GetTransform().m_position);
					}
					m_enemySleepingMenu->SetDraw(isFound);
				}

				/** 睡眠タイマーUIへ通知 */
				if (m_pbWakingUpTimerMenu)
				{
					if (isFound)
					{
						auto* sm = enemy->GetEnemyStateMachine();
						m_pbWakingUpTimerMenu->SetCurrentPBTime(sm->GetSleepTimer());
						m_pbWakingUpTimerMenu->SetTargetPosition(enemy->GetTransform().m_position);
					}
					m_pbWakingUpTimerMenu->SetDraw(isFound);
				}
			}
		);
	}


	//============================================//
	// 更新処理
	//============================================//

	void InGameUIManager::UpdateCountDown()
	{
		if (m_countDownLayout) m_countDownLayout->Update();
	}


	void InGameUIManager::UpdatePlaying()
	{
		if (m_timerLayout) m_timerLayout->Update();

		if (m_remainingChildLayout) m_remainingChildLayout->Update();

		for (auto* layout : m_searchLayouts)
		{
			if (layout) layout->Update();
		}

		if (m_enemySleepingLayout) m_enemySleepingLayout->Update();

		if (m_pbWakingUpTimerLayout) m_pbWakingUpTimerLayout->Update();

		if (m_iglooPromptLayout) m_iglooPromptLayout->Update();
	}


	void InGameUIManager::UpdateFinishing()
	{
		if (m_finishLayout) m_finishLayout->Update();
	}


	//============================================//
	// 描画処理
	//============================================//

	void InGameUIManager::RenderCountDown(RenderContext& rc)
	{
		if (m_countDownLayout) m_countDownLayout->Render(rc);
	}


	void InGameUIManager::RenderPlaying(RenderContext& rc)
	{
		if (m_timerLayout) m_timerLayout->Render(rc);

		if (m_remainingChildLayout) m_remainingChildLayout->Render(rc);

		for (auto* layout : m_searchLayouts)
		{
			if (layout) layout->Render(rc);
		}

		if (m_enemySleepingLayout) m_enemySleepingLayout->Render(rc);

		if (m_pbWakingUpTimerLayout) m_pbWakingUpTimerLayout->Render(rc);

		if (m_iglooPromptLayout) m_iglooPromptLayout->Render(rc);
	}


	void InGameUIManager::RenderFinishing(RenderContext& rc)
	{
		if (m_timerLayout) m_timerLayout->Render(rc);

		if (m_finishLayout) m_finishLayout->Render(rc);
	}


	void InGameUIManager::RenderPause(RenderContext& rc)
	{
		if (m_pauseLayout) m_pauseLayout->Render(rc);
	}


	void InGameUIManager::RenderSoundOption(RenderContext& rc)
	{
		if (m_soundOptionLayout) m_soundOptionLayout->Render(rc);
	}

	void InGameUIManager::RenderTutorial(RenderContext& rc)
	{
		if (m_tutorialLayout) m_tutorialLayout->Render(rc);
	}
}