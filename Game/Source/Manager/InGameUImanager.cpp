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

#include "Source/UI/Layout.h"

#include "Source/UI/CPReaction/CPReactionSystem.h"
#include "Source/UI/DangerArrow/DangerArrowSystem.h"
#include "Source/UI/InGameButton/InGameButtonMenu.h"
#include "Source/UI/InGameTimer/InGameTimerMenu.h"
#include "Source/UI/Menus/AchievementNotificationMenu.h"
#include "Source/UI/Menus/CountDownMenu.h"
#include "Source/UI/Menus/DebufMenu.h"
#include "Source/UI/Menus/EnemySleepingMenu.h"
#include "Source/UI/Menus/FinishMenu.h"
#include "Source/UI/Menus/IglooPromptMenu.h"
#include "Source/UI/Menus/InGameAchievementMenu.h"
#include "Source/UI/Menus/PauseScreenMenu.h"
#include "Source/UI/Menus/PBWakingUpTimerMenu.h"
#include "Source/UI/Menus/SearchMenu.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TitleEventMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"
#include "Source/UI/MiniMap/MiniMapMenu.h"
#include "Source/UI/RemainingChild/RemainingChildMenu.h"
#include "Source/UI/WpWarning/WpWarningSystem.h"


namespace app
{
	namespace
	{
		/** 睡眠中クマの探索半径 */
		constexpr float SLEEPING_ENEMY_SEARCH_RANGE = 1000.0f;
	}


	InGameUIManager* InGameUIManager::m_instance = nullptr;


	InGameUIManager::InGameUIManager()
	{}


	InGameUIManager::~InGameUIManager()
	{
		/** BattleManagerのfunctionをリセットして、dangling参照を防ぐ */
		BattleManager::GetInstance().ResetObservers();

		// パケットはunique_ptrで自動的に破棄されるため、処理はいらない
		m_searchPackets.clear();
	}


	void InGameUIManager::Initialize()
	{
		//------------------------------------------------------------
		// Layout・Menuの生成
		//------------------------------------------------------------
		// カウントダウンを生成
		ui::InitUIPacket(m_countDownPacket, "Assets/parameter/countDown/CountDown.json");
		// タイマーを生成
		ui::InitUIPacket(m_timerPacket, "Assets/parameter/timer/InGameTimer.json");
		// フィニッシュを生成
		ui::InitUIPacket(m_finishPacket, "Assets/parameter/event/FinishMenu.json");
		// 残り子ペンギン数を生成
		ui::InitUIPacket(m_remainingChildPacket, "Assets/parameter/UI/remainingChild/remainingChild.json");
		// ポーズ画面を生成
		ui::InitUIPacket(m_pausePacket, "Assets/parameter/pause/PauseScreen.json");
		// サウンドオプションを生成
		ui::InitUIPacket(m_soundOptionPacket, "Assets/parameter/sound/SoundOption.json");
		// 睡眠中クマの起床ゲージを生成
		ui::InitUIPacket(m_enemySleepingPacket, "Assets/parameter/UI/enemySleepGauge/sleepGauge.json");
		// デバフメニューを生成
		ui::InitUIPacket(m_debufPacket, "Assets/parameter/debuf/Debuf.json");

		if (auto* menu = m_debufPacket->GetMenu()) menu->SetDraw(false);

		if (auto* menu = m_enemySleepingPacket->GetMenu())
		{
			menu->SetDraw(false);
			menu->SetSleepingRate(0.0f);
		}

		// 起床中クマのPB起床ゲージを生成
		ui::InitUIPacket(m_pbWakingUpTimerPacket, "Assets/parameter/timer/PBTimer/PBWakingUpTimer.json");

		if (auto* menu = m_pbWakingUpTimerPacket->GetMenu()) menu->SetDraw(false);

		// ミニマップを生成
		ui::InitUIPacket(m_miniMapPacket, "Assets/parameter/UI/miniMap/MiniMap.json");


		// タイトルイベントを生成
		ui::InitUIPacket(m_titleEventPacket, "Assets/parameter/event/TitleEvent.json");

		if (auto* menu = m_titleEventPacket->GetMenu()) menu->SetDraw(true);

		// 達成通知を生成
		ui::InitUIPacket(m_achievementNotificationPacket, "Assets/parameter/UI/inGameAchievement/AchievementNotify.json");
		// イグループプロンプトを生成
		ui::InitUIPacket(m_iglooPromptPacket, "Assets/parameter/UI/iglooPrompt/IglooPrompt.json");


		// チュートリアルを生成
		ui::InitUIPacket(m_tutorialPacket, "Assets/parameter/tutorial/Tutorial.json");
		// 実績を生成
		ui::InitUIPacket(m_achievementPacket, "Assets/parameter/UI/inGameAchievement/InGameAchievement.json");
		// インゲームボタンを生成
		ui::InitUIPacket(m_inGameButtonPacket, "Assets/parameter/UI/inGameButton/InGameButton.json");
		// 子ペンギンリアクションシステムを生成
		m_cpReactionSystem = std::make_unique<ui::CPReactionSystem>();
		m_cpReactionSystem->Initialize();

		// WpWarningSystemを生成
		m_wpWarningSystem = std::make_unique<ui::WpWarningSystem>();
		m_wpWarningSystem->Initialize();

		// 危険矢印システムを生成
		m_dangerArrowSystem = std::make_unique<ui::DangerArrowSystem>();
		m_dangerArrowSystem->Initialize();
	}


	void InGameUIManager::SetMiniMapIconNum(ui::EnMiniMapIconType type, uint8_t num)
	{
		m_miniMapPacket->GetMenu()->SetIconNum(type, num);
	}


	void InGameUIManager::InitializeMapIcon()
	{
		m_miniMapPacket->GetMenu()->InitializeMapIcon();
	}


	void InGameUIManager::AddSearchLayout(actor::Enemy* enemy)
	{
		ui::UIPacket<ui::SearchMenu> searchPacket;
		ui::InitUIPacket(searchPacket, "Assets/parameter/search/Search.json");


		if (auto* menu = searchPacket->GetMenu())
		{
			menu->SetEnemy(enemy);
			menu->SetIsActive(true);
			menu->SetDraw(false);
		}

		m_searchPackets.push_back(std::move(searchPacket));
	}


	void InGameUIManager::RegisterObservers()
	{
		auto& bm = BattleManager::GetInstance();

		auto* daddyPenguin = bm.GetDaddyPenguin();

		// UIがnullptrでないかをチェックするラムダ
		auto CheckMenu = [](ui::MenuBase* menu)
			{
				K2_ASSERT(menu, "メニューがnullptr");
			};

		m_cpReactionSystem->SetDaddyPenguin(daddyPenguin);
		m_wpWarningSystem->SetDaddyPenguin(daddyPenguin);

		//--------------------------------------------//
		// タイマーUI通知
		//--------------------------------------------//
		bm.SetOnTimeChanged(
			[this, CheckMenu](float time)
			{
				auto* menu = m_timerPacket->GetMenu();
				CheckMenu(menu);
				menu->SetTime(time);
			}
		);

		//--------------------------------------------//
		// 残り子ペンギン数UI通知
		//--------------------------------------------//
		bm.SetOnRescuedNumChanged(
			[this, CheckMenu](int rescued, int total)
			{
				auto* menu = m_remainingChildPacket->GetMenu();
				CheckMenu(menu);
				menu->SetChildNum(rescued);
				menu->SetTotalNum(total);
			}
		);

		//--------------------------------------------//
		// 睡眠中クマUI通知
		// daddyPenguinをキャプチャしてlambda内で探索する
		//--------------------------------------------//
		bm.SetOnSleepingEnemyChanged(
			[this, daddyPenguin, CheckMenu]()
			{
				/** プレイヤー座標を基準に最近傍の睡眠中クマを探す */
				const Vector3 playerPos = daddyPenguin->GetTransform().m_position;
				auto* enemy = actor::EnemyManager::GetInstance()->GetNearestSleepingEnemy(
					playerPos,
					SLEEPING_ENEMY_SEARCH_RANGE
				);

				const bool isFound = (enemy != nullptr);

				/** 起床ゲージUIへ通知 */
				auto* sleepingMenu = m_enemySleepingPacket->GetMenu();
				CheckMenu(sleepingMenu);
				if (isFound)
				{
					auto* sm = enemy->GetEnemyStateMachine();
					/** 起床ゲージ（満タン=100）を0〜1に正規化して渡す */
					sleepingMenu->SetSleepingRate(sm->GetWakeUpGauge() / 100.0f);
					sleepingMenu->SetTargetPosition(enemy->GetTransform().m_position);
				}

				sleepingMenu->SetDraw(isFound);



				/** 睡眠タイマーUIへ通知 */
				auto* walkingMenu = m_pbWakingUpTimerPacket->GetMenu();
				CheckMenu(walkingMenu);
				if (isFound)
				{
					auto* sm = enemy->GetEnemyStateMachine();
					walkingMenu->SetCurrentPBTime(sm->GetSleepTimer());
					walkingMenu->SetTargetPosition(enemy->GetTransform().m_position);
				}
				walkingMenu->SetDraw(isFound);
			}
		);


		//--------------------------------------------//
		// ミニマップUI通知
		//--------------------------------------------//
		bm.SetOnMiniMapChanged(
			[this, daddyPenguin, CheckMenu](const ui::ActorPositions& actorPositions)
			{
				auto* menu = m_miniMapPacket->GetMenu();
				CheckMenu(menu);

				menu->SetActorPositions(
					daddyPenguin->GetTransform().m_position,
					actorPositions
				);
			}
		);
	}


	//============================================//
	// 更新処理
	//============================================//

	void InGameUIManager::UpdateCountDown()
	{
		if (m_countDownPacket) m_countDownPacket->Update();
	}


	void InGameUIManager::UpdatePlaying()
	{
		if (m_timerPacket) m_timerPacket->Update();
		if (m_remainingChildPacket) m_remainingChildPacket->Update();
		for (auto& packet : m_searchPackets)
		{
			if (packet) packet->Update();
		}
		if (m_cpReactionSystem) m_cpReactionSystem->Update();
		if (m_wpWarningSystem) m_wpWarningSystem->Update();
		if (m_dangerArrowSystem) m_dangerArrowSystem->Update();
		if (m_enemySleepingPacket) m_enemySleepingPacket->Update();
		if (m_pbWakingUpTimerPacket) m_pbWakingUpTimerPacket->Update();
		if (m_iglooPromptPacket) m_iglooPromptPacket->Update();
		if (m_miniMapPacket) m_miniMapPacket->Update();
		if (m_achievementPacket) m_achievementPacket->Update();
		if (m_achievementNotificationPacket) m_achievementNotificationPacket->Update();
		if (m_inGameButtonPacket) m_inGameButtonPacket->Update();
		if (m_debufPacket) m_debufPacket->Update();
	}


	void InGameUIManager::UpdateFinishing()
	{
		if (m_finishPacket) m_finishPacket->Update();
	}


	void InGameUIManager::UpdateAchievementHotReload()
	{
		if (m_achievementPacket) m_achievementPacket->Update();
	}


	void InGameUIManager::UpdatePause()
	{
		if (m_pausePacket) m_pausePacket->Update();
	}


	//============================================//
	// 描画処理
	//============================================//

	void InGameUIManager::RenderCountDown(RenderContext& rc)
	{
		if (m_countDownPacket) m_countDownPacket->Render(rc);
	}


	void InGameUIManager::RenderPlaying(RenderContext& rc)
	{
		for (auto& packet : m_searchPackets)
		{
			if (packet) packet->Render(rc);
		}
		if (m_enemySleepingPacket) m_enemySleepingPacket->Render(rc);
		if (m_cpReactionSystem) m_cpReactionSystem->Render(rc);
		if (m_wpWarningSystem) m_wpWarningSystem->Render(rc);
		if (m_dangerArrowSystem) m_dangerArrowSystem->Render(rc);
		if (m_pbWakingUpTimerPacket) m_pbWakingUpTimerPacket->Render(rc);
		if (m_iglooPromptPacket) m_iglooPromptPacket->Render(rc);
		if (m_miniMapPacket) m_miniMapPacket->Render(rc);
		if (m_timerPacket) m_timerPacket->Render(rc);
		if (m_remainingChildPacket) m_remainingChildPacket->Render(rc);
		if (m_achievementNotificationPacket) m_achievementNotificationPacket->Render(rc);
		if (m_inGameButtonPacket) m_inGameButtonPacket->Render(rc);
		if (m_debufPacket) m_debufPacket->Render(rc);
	}


	void InGameUIManager::RenderFinishing(RenderContext& rc)
	{
		if (m_timerPacket) m_timerPacket->Render(rc);
		if (m_finishPacket) m_finishPacket->Render(rc);
	}


	void InGameUIManager::RenderPause(RenderContext& rc)
	{
		if (m_pausePacket) m_pausePacket->Render(rc);
		if (m_achievementPacket) m_achievementPacket->Render(rc);
	}


	void InGameUIManager::RenderSoundOption(RenderContext& rc)
	{
		if (m_soundOptionPacket) m_soundOptionPacket->Render(rc);
	}


	void InGameUIManager::RenderTutorial(RenderContext& rc)
	{
		if (m_tutorialPacket) m_tutorialPacket->Render(rc);
	}
}