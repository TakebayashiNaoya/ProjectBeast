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
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"

#include "Source/UI/Layout.h"

#include "Source/UI/BearReaction/BearReactionSystem.h"
#include "Source/UI/BearReaction/BearReactionTypes.h"
#include "Source/UI/CPReaction/CPReactionSystem.h"
#include "Source/UI/DangerArrow/DangerArrowSystem.h"
#include "Source/UI/Fever/FeverIconMenu.h"
#include "Source/UI/FormationWheel/FormationWheelMenu.h"
#include "Source/UI/InGameButton/InGameButtonMenu.h"
#include "Source/UI/InGameTimer/InGameTimerMenu.h"
#include "Source/UI/Menus/AchievementNotificationMenu.h"
#include "Source/UI/Menus/CountDownMenu.h"
#include "Source/UI/Menus/DebufMenu.h"
#include "Source/UI/Menus/EnemySleepingMenu.h"
#include "Source/UI/Menus/FinishMenu.h"
#include "Source/UI/Menus/IglooPromptMenu.h"
#include "Source/UI/Menus/InGameAchievementMenu.h"
#include "Source/UI/Menus/LevelUpIconMenu.h"
#include "Source/UI/Menus/PauseScreenMenu.h"
#include "Source/UI/Menus/PBWakingUpTimerMenu.h"
#include "Source/UI/Menus/SearchMenu.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TitleEventMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"
#include "Source/UI/MiniMap/MiniMapMenu.h"
#include "Source/UI/RemainingChild/RemainingChildMenu.h"
#include "Source/UI/Ult/SpeedLineMenu.h"
#include "Source/UI/WpWarning/WpWarningSystem.h"

#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace
	{
		/** 睡眠中クマの探索半径 */
		constexpr float SLEEPING_ENEMY_SEARCH_RANGE = 1000.0f;

		/** レベルアップSEの音量倍率 */
		constexpr float LEVEL_UP_SE_VOLUME = 2.0f;
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
		ui::InitUIPacket(m_debufPacket, "Assets/parameter/UI/penguinDebuff/Debuf.json");

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
		// フィーバータイム落下アイコンを生成
		ui::InitUIPacket(m_feverIconPacket, "Assets/parameter/UI/fever/FeverIcon.json");
		// 陣形レベルアップアイコンを生成
		ui::InitUIPacket(m_levelUpIconPacket, "Assets/parameter/UI/levelUp/LevelUpIcon.json");
		// 陣形/ウルトのボタン表示を生成
		ui::InitUIPacket(m_formationWheelPacket, "Assets/parameter/UI/formationWheel/FormationWheel.json");

		ui::InitUIPacket(m_speedLinePacket, "Assets/parameter/UI/ult/SpeedLine.json");

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


	void InGameUIManager::SetAchievementPositionOffsetY(float offsetY)
	{
		if (!m_achievementPacket) return;

		if (auto* menu = m_achievementPacket->GetMenu())
		{
			menu->SetPositionOffsetY(offsetY);
		}
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


	void InGameUIManager::UpdateTimerAndScoreOnly()
	{
		if (m_timerPacket) m_timerPacket->Update();
		if (m_remainingChildPacket) m_remainingChildPacket->Update();
	}


	void InGameUIManager::RenderTimerAndScoreOnly(RenderContext& rc)
	{
		if (m_timerPacket) m_timerPacket->Render(rc);
		if (m_remainingChildPacket) m_remainingChildPacket->Render(rc);
	}


	void InGameUIManager::InitializeReactionSystem(const uint8_t enemyNum)
	{
		// クマのリアクションシステムを生成
		m_bearReactionSystem = std::make_unique<ui::BearReactionSystem>();
		m_bearReactionSystem->SetReactionNum(enemyNum);
		m_bearReactionSystem->Initialize();
	}


	void InGameUIManager::RegisterObservers()
	{
		auto& bm = BattleManager::GetInstance();

		auto* daddyPenguin = bm.GetDaddyPenguin();

		// スタミナゲージUI通知用にキャッシュしておく
		m_daddyPenguin = daddyPenguin;

		// UIがnullptrでないかをチェックするラムダ
		auto CheckMenu = [](ui::MenuBase* menu)
			{
				K2_ASSERT(menu, "メニューがnullptr");
			};

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
		// クマのリアクションUI通知
		//--------------------------------------------//
		bm.SetOnBearReactionChanged(
			[this, daddyPenguin]()
			{
				const auto& enemies = actor::EnemyManager::GetInstance()->GetEnemies();

				for (uint8_t i = 0; i < enemies.size(); ++i)
				{
					auto* it = enemies.at(i);
					bool isReturning = it->GetEnemyStateMachine()->IsReturnHome();
					bool isDebuff = it->GetEnemyStateMachine()->IsDebuffReturnHome();
					bool isChasing = it->GetEnemyStateMachine()->IsChasing();

					auto type = ui::EnBearReactionType::None;

					if (isDebuff)
					{
						type = ui::EnBearReactionType::Debuff;
					}
					else if (isReturning)
					{
						type = ui::EnBearReactionType::Bed;
					}
					else if (isChasing)
					{
						type = ui::EnBearReactionType::Tongue;
					}

					// クマのリアクションUIに位置を通知
					m_bearReactionSystem->SetReaction(
						i,
						it->GetTransform().m_position,
						daddyPenguin->GetTransform(),
						type
					);
				}
			}
		);

		//--------------------------------------------//
		// 子ペンギンリアクションUI通知
		// タイプの確定は呼び出し側（ChildPenguinManagerや各AIController）の責務。
		// ここではSystemへの反映のみを行う。
		//--------------------------------------------//
		bm.SetOnCPReactionChanged(
			[this](actor::ChildPenguin* penguin, ui::EnCPReactionType type, ui::EnCPReactionPriority priority)
			{
				if (m_cpReactionSystem) m_cpReactionSystem->SetTarget(penguin, type, priority);
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


		//--------------------------------------------//
		// 陣形レベルアップUI通知
		//--------------------------------------------//
		bm.SetOnFormationLevelUp(
			[this](int)
			{
				if (!m_levelUpIconPacket) return;

				auto* menu = m_levelUpIconPacket->GetMenu();
				if (!menu) return;

				menu->Play();

				// レベルアップSEを再生。
				SoundManager::Get().PlaySE(enSoundKind_LevelUp, LEVEL_UP_SE_VOLUME, false, false, enSoundPriority_Hight);
			}
		);

		//--------------------------------------------//
		// 渦潮UI通知
		//--------------------------------------------//
		bm.SetOnWpWarningChanged(
			[this, daddyPenguin, CheckMenu](std::vector<Vector3> whirlpoolPositions)
			{
				m_wpWarningSystem->SetDaddyTRS(daddyPenguin->GetTransform());
				m_wpWarningSystem->SetWhirlpoolPositions(whirlpoolPositions);
				m_wpWarningSystem->UpdateDrawFlags();
			}
		);


		//--------------------------------------------//
		// スピードアップ中の通知
		//--------------------------------------------//
		bm.SetOnSpeedLineChanged(
			[this, CheckMenu](bool isSpeedUp)
			{
				const float acceleration = isSpeedUp ? 1.0f : 0.0f;

				auto* menu = m_speedLinePacket->GetMenu();
				CheckMenu(menu);
				menu->SetActive(isSpeedUp);
				menu->SetAcceleration(acceleration);
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
		if (m_bearReactionSystem) m_bearReactionSystem->Update();
		if (m_wpWarningSystem) m_wpWarningSystem->Update();
		if (m_dangerArrowSystem) m_dangerArrowSystem->Update();
		if (m_enemySleepingPacket) m_enemySleepingPacket->Update();
		if (m_pbWakingUpTimerPacket) m_pbWakingUpTimerPacket->Update();
		if (m_iglooPromptPacket) m_iglooPromptPacket->Update();
		if (m_miniMapPacket) m_miniMapPacket->Update();
		if (m_achievementPacket) m_achievementPacket->Update();
		if (m_achievementNotificationPacket) m_achievementNotificationPacket->Update();
		if (m_speedLinePacket) m_speedLinePacket->Update();

		if (m_achievementPacket)
		{
			bool isShowing = false;
			if (m_achievementNotificationPacket)
			{
				if (auto* notifyMenu = m_achievementNotificationPacket->GetMenu())
				{
					isShowing = notifyMenu->IsShowing();
				}
			}

			if (auto* listMenu = m_achievementPacket->GetMenu())
			{
				listMenu->SetDraw(!isShowing);
			}
		}

		// ジャンプ・スライドのスタミナ状態をボタンUIへ通知する（毎フレーム）
		if (m_daddyPenguin && m_inGameButtonPacket)
		{
			if (auto* menu = m_inGameButtonPacket->GetMenu())
			{
				if (auto* sm = m_daddyPenguin->GetStateMachine())
				{
					menu->SetJumpStaminaInfo(sm->GetJumpStaminaRatio(), !sm->CanUseJump());
					menu->SetSlideStaminaInfo(sm->GetSlideStaminaRatio(), !sm->CanUseSlide());
				}
			}
		}

		if (m_inGameButtonPacket) m_inGameButtonPacket->Update();
		if (m_formationWheelPacket) m_formationWheelPacket->Update();
		if (m_debufPacket) m_debufPacket->Update();
		if (m_feverIconPacket) m_feverIconPacket->Update();

		// 陣形レベルアップアイコンを親ペンギンの頭上へ追従させる（再生中に位置がずれないよう毎フレーム更新）
		if (m_levelUpIconPacket)
		{
			if (m_daddyPenguin)
			{
				if (auto* menu = m_levelUpIconPacket->GetMenu())
				{
					menu->SetTargetPosition(m_daddyPenguin->GetTransform().m_position);
				}
			}
			m_levelUpIconPacket->Update();
		}
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

		if (m_achievementPacket)
		{
			if (auto* listMenu = m_achievementPacket->GetMenu())
			{
				listMenu->SetDraw(true);
			}
		}
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
		if (m_speedLinePacket) m_speedLinePacket->Render(rc);
		for (auto& packet : m_searchPackets)
		{
			if (packet) packet->Render(rc);
		}
		if (m_enemySleepingPacket) m_enemySleepingPacket->Render(rc);
		if (m_bearReactionSystem) m_bearReactionSystem->Render(rc);
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
		if (m_formationWheelPacket) m_formationWheelPacket->Render(rc);
		if (m_debufPacket) m_debufPacket->Render(rc);
		if (m_feverIconPacket) m_feverIconPacket->Render(rc);
		if (m_levelUpIconPacket) m_levelUpIconPacket->Render(rc);
	}


	void InGameUIManager::RenderAchievementInPlaying(RenderContext& rc)
	{
		if (m_achievementPacket) m_achievementPacket->Render(rc);
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
