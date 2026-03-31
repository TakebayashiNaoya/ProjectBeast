/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"
#include "Source/Util/JsonConverter.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/CameraController.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Noise/NoiseManager.h"

#include "Source/Manager/BattleManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"

#include "Source/UI/Layout.h"
#include "Source/UI/CountDownMenu.h"
#include "Source/UI/InGameTimerMenu.h"
#include "Source/UI/FinishMenu.h"
#include "Source/UI/RemainingChildMenu.h"
#include "Source/UI/PauseScreenMenu.h"
#include "Source/UI/SoundOptionMenu.h"
#include "Source/UI/SearchMenu.h"

#include "Source/Scene/SceneManager.h"
#include "TitleScene.h"

#include "Source/UI/EnemySleepingMenu.h"
#include "Source/UI/PBWakingUpTimerMenu.h"

#include <random>


namespace app
{
	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
		// UI
		delete m_countDownLayout;
		delete m_timerLayout;
		delete m_finishLayout;
		delete m_remainingChildLayout;
		delete m_pauseLayout;
		delete m_soundOptionLayout;
		for (auto* layout : m_searchLayouts)
		{
			delete layout;
		}
		m_searchLayouts.clear();
		delete m_enemySleepingLayout;
		delete m_pbWakingUpTimerLayout;

		// アクター
		actor::StageSystem::DestroyInstance();
		actor::EnemyManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();

		DeleteGO(m_ocean);

		// マネージャー
		BattleManager::DestroyInstance();
		ScoreManager::DestroyInstance();
		TimeManager::DestroyInstance();
	}


	bool InGameScene::Start()
	{
		// マネージャー生成
		app::core::ParameterManager::CreateInstance();
		BattleManager::CreateInstance();
		ScoreManager::CreateInstance();
		TimeManager::CreateInstance();

		// アクター系シングルトン生成
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyManager::CreateInstance();

		// UI レイアウト生成
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

		m_pauseLayout = new ui::Layout();
		m_pauseLayout->Initialize<ui::PauseScreenMenu>(
			"Assets/parameter/pause/PauseScreen.json"
		);
		m_pauseMenu = m_pauseLayout->GetMenu<ui::PauseScreenMenu>();


		// サウンドオプション画面のセットアップ（TitleSceneと同じパスを使用）
		m_soundOptionLayout = new ui::Layout;
		m_soundOptionLayout->Initialize<ui::SoundOptionMenu>("Assets/parameter/sound/SoundOption.json");
		m_soundOptionMenu = m_soundOptionLayout->GetMenu<ui::SoundOptionMenu>();

		// クマの起床ゲージUI
		m_enemySleepingLayout = new ui::Layout();
		m_enemySleepingLayout->Initialize<ui::EnemySleepingMenu>(
			"Assets/parameter/UI/enemySleepGauge/sleepGauge.json"
		);
		{
			auto* menu = m_enemySleepingLayout->GetMenu<ui::EnemySleepingMenu>();
			if (menu) {
				menu->SetDraw(false);
				menu->SetSleepingRate(0.0f);
			}
		}

		// クマの睡眠タイマーUI
		m_pbWakingUpTimerLayout = new ui::Layout();
		m_pbWakingUpTimerLayout->Initialize<ui::PBWakingUpTimerMenu>(
			"Assets/parameter/timer/PBWakingUpTimer.json"
		);
		{
			auto* menu = m_pbWakingUpTimerLayout->GetMenu<ui::PBWakingUpTimerMenu>();
			if (menu) {
				menu->SetDraw(false);
				menu->SetCurrentPBTime(0.0f);
			}
		}

		// ロードフェーズ開始
		m_loadPhase = LoadPhase::Stage;
		m_childIndex = 0;

		return true;
	}


	void InGameScene::Update()
	{
		//------------------------------------------------------------
		// ロードフェーズ（既存ロジックそのまま）
		//------------------------------------------------------------
		switch (m_loadPhase)
		{
		case LoadPhase::Stage:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/stage/stageObject.json");
			actor::StageSystem::GetInstance()->CreateStageObject(json);
			m_loadPhase = LoadPhase::Daddy;
			break;
		}

		case LoadPhase::Daddy:
			m_daddyPenguin = new actor::DaddyPenguin();
			m_daddyPenguin->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
			m_daddyPenguin->StartWrapper();
			m_loadPhase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
			if (m_childIndex < CHILD_PENGUIN_NUM)
			{
				static std::random_device rd;
				static std::mt19937 gen(rd());
				static std::uniform_real_distribution<float> dis(-2000.0f, 2000.0f);

				Vector3 pos = Vector3(dis(gen), 0.0f, dis(gen));
				actor::ChildPenguinManager::GetInstance()->CreateChildPenguin(1);

				const auto& children = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();
				auto* child = children.back();

				if (m_childIndex < 50)
				{
					child->SetChildPenguinType(app::actor::EnChildPenguinType::Serious);
				}
				else if (m_childIndex < 100)
				{
					child->SetChildPenguinType(app::actor::EnChildPenguinType::Clingy);
				}

				child->SetPosition(pos);
				child->GetStateMachine()->SetPosition(pos);
				child->StartWrapper();
				++m_childIndex;
			}
			else
			{
				auto* manager = app::actor::ChildPenguinManager::GetInstance();
				manager->SetDaddyPenguin(m_daddyPenguin);

				// ステージ上の総ペンギン数をセット
				ScoreManager::GetInstance().SetTotalCount(CHILD_PENGUIN_NUM);

				m_loadPhase = LoadPhase::Enemy;
			}
			break;

		case LoadPhase::Enemy:
		{
			nlohmann::json json;
			// プロジェクト内の実際のパスに合わせてください
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/character/enemy/EnemyLayout.json");

			// マネージャーにJSONを渡して一括生成させる
			actor::EnemyManager::GetInstance()->LoadEnemies(json);

			// ↓ ここから追加
			for (auto* enemy : actor::EnemyManager::GetInstance()->GetEnemies())
			{
				auto* layout = new ui::Layout();
				layout->Initialize<ui::SearchMenu>("Assets/parameter/search/Search.json");

				auto* menu = layout->GetMenu<ui::SearchMenu>();
				menu->SetEnemy(enemy);
				menu->SetIsActive(true);

				m_searchLayouts.push_back(layout);
				m_searchMenus.push_back(menu);
			}

			m_loadPhase = LoadPhase::Camera;
			break;
		}

		case LoadPhase::Camera:
		{
			camera::CameraSteering::Config config;
			m_cameraSteering.SetConfig(config);
			m_cameraSteering.SetTargetCharacter(m_daddyPenguin);

			auto gameCamera = std::make_shared<camera::GameCamera>();
			camera::CameraManager::Get().Register(camera::GameCamera::ID(), gameCamera);
			camera::CameraManager::Get().SwitchCamera(camera::GameCamera::ID());
			m_loadPhase = LoadPhase::Ocean;
			break;
		}

		case LoadPhase::Ocean:
			m_ocean = NewGO<Ocean>(0);
			m_loadPhase = LoadPhase::Done;

			// ロード完了 → カウントダウン開始
			if (m_countDownMenu)
			{
				m_countDownMenu->SetCountDownStartFlag(true);
			}
			break;

		case LoadPhase::Done:
			// ゲームフェーズへ移譲
			UpdateGamePhase();
			break;

		default:
			break;
		}
	}


	void InGameScene::UpdateGamePhase()
	{
		// カメラは常に更新
		auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(camera::GameCamera::ID());
		if (gameCamera)
		{
			camera::CameraData data = gameCamera->GetCameraData();
			m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(data);
		}

		// ステージは常に更新
		actor::StageSystem::GetInstance()->Update();

		switch (m_gamePhase)
		{
			//------------------------------------------------------------
			// カウントダウン
			//------------------------------------------------------------
		case GamePhase::CountDown:
		{
			// BattleManager にゲーム非アクティブを伝える
			BattleManager::GetInstance().SetIsActive(false);

			// AI・入力は動かさないが、描画用の行列更新だけ行う
			if (m_daddyPenguin) m_daddyPenguin->UpdateModelOnly();
			actor::ChildPenguinManager::GetInstance()->UpdateModelOnly();
			actor::EnemyManager::GetInstance()->UpdateModelOnly();

			// カウントダウン UI 更新
			if (m_countDownLayout) m_countDownLayout->Update();

			// カウントダウン完了 → Playing へ
			if (m_countDownMenu && m_countDownMenu->IsCountDownFinished())
			{
				m_gamePhase = GamePhase::Playing;
				BattleManager::GetInstance().SetIsActive(true);

				// Playing に切り替わった直後のフレームで一瞬表示されないよう明示的に非表示にする
				if (auto* menu = m_enemySleepingLayout->GetMenu<ui::EnemySleepingMenu>())
				{
					menu->SetDraw(false);
				}
				if (auto* menu = m_pbWakingUpTimerLayout->GetMenu<ui::PBWakingUpTimerMenu>())
				{
					menu->SetDraw(false);
				}
			}
			break;
		}

		//------------------------------------------------------------
		// プレイ中
		//------------------------------------------------------------
		case GamePhase::Playing:
		{
			if (g_pad[0]->IsTrigger(enButtonStart))
			{
				SceneManager::GetInstance()->SetPause(true);
			}

			// プレイヤー・子ペンギン・シロクマ の更新
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			actor::ChildPenguinManager::GetInstance()->Update();
			actor::EnemyManager::GetInstance()->Update();

			// タイマー UI 更新
			if (m_timerLayout) m_timerLayout->Update();
			// 探索 UI 更新
			for (auto* layout : m_searchLayouts)
			{
				if (layout) layout->Update();
			}

			// 残り子ペンギン数 UI 更新
			if (m_remainingChildLayout) {
				auto* menu = m_remainingChildLayout->GetMenu<ui::RemainingChildMenu>();
				if (menu) {
					const int childNum = actor::ChildPenguinManager::GetInstance()->GetFollowersNum();
					menu->SetChildNum(childNum);
				}

				m_remainingChildLayout->Update();
			}

			// クマの起床ゲージUI・睡眠タイマーUI 更新
			// 一番近くで寝ているエネミーを1回だけ探して両メニューに渡す
			if (m_enemySleepingLayout && m_pbWakingUpTimerLayout)
			{
				constexpr float MAX_RANGE = 1000.0f;
				constexpr float MAX_RANGE_SQ = MAX_RANGE * MAX_RANGE;

				// 一番近くで寝ているエネミーを探す
				actor::Enemy* nearestSleepingEnemy = nullptr;
				float minDistSq = MAX_RANGE_SQ;
				const Vector3 playerPosition = m_daddyPenguin->GetTransform().m_position;

				for (auto* enemy : actor::EnemyManager::GetInstance()->GetEnemies())
				{
					auto* stateMachine = enemy->GetEnemyStateMachine();

					// クールダウン（睡眠）状態でなければ対象外
					if (!stateMachine->IsCoolDown()) {
						continue;
					}

					Vector3 d = playerPosition - enemy->GetTransform().m_position;
					const float dSq = d.LengthSq();

					// 指定距離以上なら処理しない
					if (dSq > MAX_RANGE_SQ) {
						continue;
					}

					if (dSq < minDistSq) {
						minDistSq = dSq;
						nearestSleepingEnemy = enemy;
					}
				}

				const bool isFind = (nearestSleepingEnemy != nullptr);

				// 起床ゲージUI に渡す
				{
					auto* menu = m_enemySleepingLayout->GetMenu<ui::EnemySleepingMenu>();
					if (menu) {
						if (isFind) {
							auto* sm = nearestSleepingEnemy->GetEnemyStateMachine();
							// 起床ゲージ（満タン=1.0f、0=起きる）を0〜1に正規化して渡す
							menu->SetSleepingRate(sm->GetWakeUpGauge() / 100.0f);
							menu->SetTargetPosition(nearestSleepingEnemy->GetTransform().m_position);
						}
						menu->SetDraw(isFind);
					}
					m_enemySleepingLayout->Update();
				}

				// 睡眠タイマーUI に渡す
				{
					auto* menu = m_pbWakingUpTimerLayout->GetMenu<ui::PBWakingUpTimerMenu>();
					if (menu) {
						if (isFind) {
							auto* sm = nearestSleepingEnemy->GetEnemyStateMachine();
							menu->SetCurrentPBTime(sm->GetSleepTimer());
							menu->SetTargetPosition(nearestSleepingEnemy->GetTransform().m_position);
						}
						menu->SetDraw(isFind);
					}
					m_pbWakingUpTimerLayout->Update();
				}
			}

			BattleManager::GetInstance().Update();
			TimeManager::GetInstance().Update();

			// ノイズリストをクリア
			NoiseManager::GetInstance().ClearNoises();

			// 終了判定
			if (BattleManager::GetInstance().GetBattleState() != BattleManager::EnBattleState::Playing)
			{
				// FINISH 演出開始
				if (m_finishMenu) m_finishMenu->StartFinish();

				m_gamePhase = GamePhase::Finishing;
			}
			break;
		}

		//------------------------------------------------------------
		// FINISH 演出
		//------------------------------------------------------------
		case GamePhase::Finishing:
		{
			// FINISH UI 更新
			if (m_finishLayout) m_finishLayout->Update();

			// 演出終了 → リザルトへ
			if (m_finishMenu && m_finishMenu->IsFinished())
			{
				m_nextScene = true;
			}
			break;
		}
		}
	}


	void InGameScene::PauseUpdate()
	{
		switch (m_pauseState)
		{
			//------------------------------------------------------------
			// ポーズメニュー
			//------------------------------------------------------------
		case PauseState::Pause:
		{
			if (!m_pauseMenu) break;

			m_pauseMenu->Update();
			m_pauseMenu->EnterType();

			// ゲームに戻る
			if (m_pauseMenu->IsRetry())
			{
				m_pauseMenu->IsRetry(false);
				SceneManager::GetInstance()->SetPause(false);
			}
			// サウンドオプションへ
			else if (m_pauseMenu->IsSound())
			{
				m_pauseMenu->IsSound(false);
				m_pauseState = PauseState::SoundOption;
			}
			// タイトルへ戻る
			else if (m_pauseMenu->IsGoTitle())
			{
				m_pauseMenu->IsGoTitle(false);
				SceneManager::GetInstance()->SetPause(false);
				m_goTitle = true;  // RequesutScene で拾う
			}
			break;
		}

		//------------------------------------------------------------
		// サウンドオプション（ポーズ中のサブ画面）
		//------------------------------------------------------------
		case PauseState::SoundOption:
		{
			if (m_soundOptionLayout)
			{
				m_soundOptionLayout->Update();
			}

			// Bボタンでポーズ画面に戻る（IsBack()ではなく直接判定）
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				m_pauseState = PauseState::Pause;
			}
			break;
		}
		}
	}


	void InGameScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);

		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
		actor::EnemyManager::GetInstance()->Render(rc);

		// ポーズ中の描画（既存のif文の後に追加）
		if (SceneManager::GetInstance()->IsPause())
		{
			switch (m_pauseState)
			{
			case PauseState::Pause:
				if (m_pauseLayout) m_pauseLayout->Render(rc);
				break;
			case PauseState::SoundOption:
				if (m_soundOptionLayout) m_soundOptionLayout->Render(rc);
				break;
			}
		}

		// UI 描画
		if (m_loadPhase == LoadPhase::Done)
		{
			switch (m_gamePhase)
			{
			case GamePhase::CountDown:
				if (m_countDownLayout) m_countDownLayout->Render(rc);
				break;
			case GamePhase::Playing:
				if (m_timerLayout) m_timerLayout->Render(rc);
				if (m_remainingChildLayout) m_remainingChildLayout->Render(rc);
				for (auto* layout : m_searchLayouts)
				{
					if (layout) layout->Render(rc);
				}
				if (m_enemySleepingLayout) m_enemySleepingLayout->Render(rc);
				if (m_pbWakingUpTimerLayout) m_pbWakingUpTimerLayout->Render(rc);
				break;
			case GamePhase::Finishing:
				if (m_timerLayout)  m_timerLayout->Render(rc);
				if (m_finishLayout) m_finishLayout->Render(rc);
				break;
			}
		}
	}


	bool InGameScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		// タイトルへ戻る
		if (m_goTitle)
		{
			id = TitleScene::ID();
			waitTime = 0.5f;
			return true;
		}
		// リザルトへ（既存）
		if (m_nextScene)
		{
			id = ResultScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}
}