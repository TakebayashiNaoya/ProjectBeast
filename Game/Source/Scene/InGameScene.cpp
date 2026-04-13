/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"

#include "Source/Sound/SoundManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Util/JsonConverter.h"

#include "Source/Manager/BattleManager.h"
#include "Source/Manager/InGameUIManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"

#include "Source/Achivement/AchievementManager.h"

#include "Source/UI/CountDownMenu.h"
#include "Source/UI/FinishMenu.h"
#include "Source/UI/PauseScreenMenu.h"
#include "Source/UI/SoundOptionMenu.h"
#include "Source/UI/TutorialMenu.h"

#include "Source/Scene/SceneManager.h"
#include "TitleScene.h"

#include "Source/Actor/Stage/WhirlpoolManager.h"
#include <random>


namespace app
{
	namespace
	{
		/** 子ペンギンのスポーン半径 */
		constexpr float CHILD_SPAWN_RADIUS = 3000.0f;

		/** タイプ別の生成数 */
		constexpr int SERIOUS_NUM = 20;
		constexpr int CLINGY_NUM = 20;
		constexpr int NAUGHTY_NUM = 20;
		constexpr int CLUMSY_NUM = 20;
		constexpr int CARING_NUM = 20;
	}


	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
		/** UIManager（デストラクタでBattleManagerのfunctionもリセットされる） */
		delete m_uiManager;

		/** アクター */
		delete m_daddyPenguin;
		actor::EnemyManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		actor::WhirlpoolManager::DestroyInstance();

		DeleteGO(m_skyCube);
		DeleteGO(m_ocean);

		/** マネージャー */
		BattleManager::DestroyInstance();
		ScoreManager::DestroyInstance();
		TimeManager::DestroyInstance();


		if (m_goTitle) {
			if (app::achievement::AchievementManager::GetInstance()) {
				app::achievement::AchievementManager::DestroyInstance();
			}
		}
	}


	bool InGameScene::Start()
	{
		/** マネージャー生成 */
		app::core::ParameterManager::CreateInstance();
		BattleManager::CreateInstance();
		ScoreManager::CreateInstance();
		TimeManager::CreateInstance();

		app::achievement::AchievementManager::CreateInstance();
		app::achievement::AchievementManager::GetInstance()->Start();

		/** アクター系シングルトン生成 */
		actor::StageSystem::CreateInstance();
		actor::WhirlpoolManager::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyManager::CreateInstance();


		/** UIManager生成（Layoutの生成はDaddyPenguin生成後のInitializeで行う） */
		m_uiManager = new InGameUIManager();

		/** ロードフェーズ開始 */
		m_loadPhase = LoadPhase::Stage;
		m_childIndex = 0;

		return true;
	}


	void InGameScene::Update()
	{
		//------------------------------------------------------------
		// ロードフェーズ
		//------------------------------------------------------------
		switch (m_loadPhase)
		{
		case LoadPhase::Stage:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/stage/stageObject.json");
			actor::StageSystem::GetInstance()->CreateStageObject(json);
			actor::WhirlpoolManager::GetInstance()->StartWrapper();
			m_loadPhase = LoadPhase::StageWait;
			break;
		}

		case LoadPhase::StageWait:
		{
			/** ステージの非同期モデルロードと物理コリジョン登録が完了するまで待つ */
			actor::StageSystem::GetInstance()->Update();
			if (actor::StageSystem::GetInstance()->IsAllLoaded())
			{
				m_loadPhase = LoadPhase::Daddy;
			}
			break;
		}

		case LoadPhase::Daddy:
			m_daddyPenguin = new actor::DaddyPenguin();
			m_daddyPenguin->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
			m_daddyPenguin->StartWrapper();

			/** DaddyPenguin生成後にUIManagerを初期化（睡眠クマ探索のキャプチャに使用） */
			m_uiManager->Initialize(m_daddyPenguin);

			m_loadPhase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
		{
			/** タイプ別数とスポーン半径を指定して一括生成 */
			actor::ChildPenguinManager::GetInstance()->CreateChildPenguins(
				SERIOUS_NUM,
				CLINGY_NUM,
				NAUGHTY_NUM,
				CLUMSY_NUM,
				CARING_NUM,
				CHILD_SPAWN_RADIUS
			);

			auto* manager = actor::ChildPenguinManager::GetInstance();
			manager->SetDaddyPenguin(m_daddyPenguin);

			/** ステージ上の総ペンギン数をセット */
			const int totalNum = SERIOUS_NUM + CLINGY_NUM + NAUGHTY_NUM + CLUMSY_NUM + CARING_NUM;
			ScoreManager::GetInstance().SetTotalCount(totalNum);

			m_loadPhase = LoadPhase::Enemy;
			break;
		}

		case LoadPhase::Enemy:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/character/enemy/EnemyLayout.json");
			actor::EnemyManager::GetInstance()->LoadEnemies(json);

			/** エネミー1体につき探索UIを1つ生成 */
			for (auto* enemy : actor::EnemyManager::GetInstance()->GetEnemies())
			{
				m_uiManager->AddSearchLayout(enemy);
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

			/**
			 * NOTE:SkyCubeは後で生み出す場所を変えるかもしれない。
			 */
			m_skyCube = NewGO<SkyCube>(0);
			m_skyCube->SetType(enSkyCubeType_Day);
			m_skyCube->SetScale(Vector3(450.0f, 450.0f, 450.0f));
			m_skyCube->SetLuminance(0.8f);

			m_ocean = NewGO<Ocean>(0);
			m_loadPhase = LoadPhase::Done;

			/** ロード完了 → カウントダウン開始 */
			if (m_uiManager->GetCountDownMenu())
			{
				m_uiManager->GetCountDownMenu()->SetIsDelay(true);
				SoundManager::Get().PlayBGM(enSoundKind_InGame);
			}
			break;

		case LoadPhase::Done:
			/** ゲームフェーズへ移譲 */
			UpdateGamePhase();
			break;

		default:
			break;
		}
	}


	void InGameScene::UpdateGamePhase()
	{
		/** カメラは常に更新 */
		auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(camera::GameCamera::ID());
		if (gameCamera)
		{
			camera::CameraData data = gameCamera->GetCameraData();
			m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(data);
		}

		/** ステージは常に更新 */
		actor::StageSystem::GetInstance()->Update();
		actor::WhirlpoolManager::GetInstance()->UpdateWrapper();

		switch (m_gamePhase)
		{
			//------------------------------------------------------------
			// カウントダウン
			//------------------------------------------------------------
		case GamePhase::CountDown:
		{
			/** BattleManager にゲーム非アクティブを伝える */
			BattleManager::GetInstance().SetIsActive(false);

			/** AI・入力は動かさないが、描画用の行列更新だけ行う */
			if (m_daddyPenguin) m_daddyPenguin->UpdateModelOnly();
			actor::ChildPenguinManager::GetInstance()->UpdateModelOnly();
			actor::EnemyManager::GetInstance()->UpdateModelOnly();

			/** カウントダウン UI 更新 */
			m_uiManager->UpdateCountDown();

			/** カウントダウン中のSE制御 */
			auto* countDownMenu = m_uiManager->GetCountDownMenu();
			if (countDownMenu)
			{
				ui::EnCountDownType currentType = countDownMenu->GetCurrentCountType();

				if (currentType != m_lastCountType)
				{
					switch (currentType)
					{
					case ui::EnCountDownType::Third:
					case ui::EnCountDownType::Second:
					case ui::EnCountDownType::First:
						SoundManager::Get().PlaySE(enSoundKind_CountDown);
						break;

					case ui::EnCountDownType::GO:
						SoundManager::Get().PlaySE(enSoundKind_GameStart);
						break;

					default:
						break;
					}

					m_lastCountType = currentType;
				}
			}

			/** カウントダウン終了判定 */
			if (countDownMenu && countDownMenu->IsCountDownFinished())
			{
				m_gamePhase = GamePhase::Playing;
				BattleManager::GetInstance().SetIsActive(true);
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

			/** プレイヤー・子ペンギン・シロクマ の更新 */
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			actor::ChildPenguinManager::GetInstance()->Update();
			actor::EnemyManager::GetInstance()->Update();

			/** インゲームUI 更新 */
			m_uiManager->UpdatePlaying();

			BattleManager::GetInstance().Update();
			TimeManager::GetInstance().Update();

			app::achievement::AchievementManager::GetInstance()->Update();

			/** ノイズリストをクリア */
			NoiseManager::GetInstance().ClearNoises();

			/** 終了判定 */
			if (BattleManager::GetInstance().GetBattleState() != BattleManager::EnBattleState::Playing)
			{
				/** FINISH 演出開始 */
				auto* finishMenu = m_uiManager->GetFinishMenu();
				if (finishMenu) finishMenu->StartFinish();

				m_gamePhase = GamePhase::Finishing;
			}
			break;
		}

		//------------------------------------------------------------
		// FINISH 演出
		//------------------------------------------------------------
		case GamePhase::Finishing:
		{
			/** FINISH UI 更新 */
			m_uiManager->UpdateFinishing();

			/** ホイッスルは演出開始時に1回だけ鳴らす */
			if (!m_isWhistlePlayed)
			{
				SoundManager::Get().PlaySE(enSoundKind_Whistle, false);
				m_isWhistlePlayed = true;
			}

			/** 演出終了 → リザルトへ */
			auto* finishMenu = m_uiManager->GetFinishMenu();
			if (finishMenu && finishMenu->IsFinished())
			{
				SoundManager::Get().StopBGM();
				m_nextScene = true;
				ResultScene::SetResult(
					TimeManager::GetInstance().GetCurTime(),
					actor::ChildPenguinManager::GetInstance()->GetRescuedNum()
				);
			}
			break;
		}
		}
	}


	void InGameScene::PauseUpdate()
	{
		/** ポーズ開始フレームに1回だけ全SEを停止する */
		if (!m_isPauseEntered)
		{
			SoundManager::Get().StopAllSE();
			m_isPauseEntered = true;
		}

		switch (m_pauseState)
		{
			//------------------------------------------------------------
			// ポーズメニュー
			//------------------------------------------------------------
		case PauseState::Pause:
		{
			auto* pauseMenu = m_uiManager->GetPauseMenu();
			if (!pauseMenu) break;

			pauseMenu->Update();
			pauseMenu->EnterType();

			/** ゲームに戻る */
			if (pauseMenu->IsRetry())
			{
				pauseMenu->IsRetry(false);
				SceneManager::GetInstance()->SetPause(false);
				/** ポーズ解除時にフラグをリセットする */
				m_isPauseEntered = false;
			}
			/** サウンドオプションへ */
			else if (pauseMenu->IsSound())
			{
				pauseMenu->IsSound(false);
				m_pauseState = PauseState::SoundOption;
			}
			/** ルール画面へ */
			else if (pauseMenu->IsRule())
			{
				pauseMenu->IsRule(false);
				auto* tutorialMenu = m_uiManager->GetTutorialMenu();
				if (tutorialMenu)
				{
					tutorialMenu->SetClosed(false);
					tutorialMenu->InitializeLogic();
				}
				m_pauseState = PauseState::Tutorial;
			}
			/** タイトルへ戻る */
			else if (pauseMenu->IsGoTitle())
			{
				pauseMenu->IsGoTitle(false);
				SceneManager::GetInstance()->SetPause(false);
				/** ポーズ解除時にフラグをリセットする */
				m_isPauseEntered = false;
				m_goTitle = true;
			}
			break;
		}

		//------------------------------------------------------------
		// サウンドオプション（ポーズ中のサブ画面）
		//------------------------------------------------------------
		case PauseState::SoundOption:
		{
			auto* soundOptionMenu = m_uiManager->GetSoundOptionMenu();
			if (soundOptionMenu)
			{
				soundOptionMenu->Update();
			}

			/** Bボタンでポーズ画面に戻る */
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				m_pauseState = PauseState::Pause;
			}
			break;
		}

		//------------------------------------------------------------
		// チュートリアル画面（ポーズ中のサブ画面）
		//------------------------------------------------------------
		case PauseState::Tutorial:
		{
			auto* tutorialMenu = m_uiManager->GetTutorialMenu();
			if (tutorialMenu)
			{
				tutorialMenu->Update();

				/** TutorialMenu 内で Bボタンが押されたら閉じる */
				if (tutorialMenu->IsClosed())
				{
					tutorialMenu->SetClosed(false);
					m_pauseState = PauseState::Pause;
				}
			}
			break;
		}
		}
	}


	void InGameScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);
		actor::WhirlpoolManager::GetInstance()->RenderWrapper(rc);

		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
		actor::EnemyManager::GetInstance()->Render(rc);

		/** ポーズ中の描画 */
		if (SceneManager::GetInstance()->IsPause())
		{
			switch (m_pauseState)
			{
			case PauseState::Pause:
				m_uiManager->RenderPause(rc);
				break;
			case PauseState::SoundOption:
				m_uiManager->RenderSoundOption(rc);
				break;
			case PauseState::Tutorial:
				m_uiManager->RenderTutorial(rc);
				break;
			}
			return;
		}

		/** UI 描画（ポーズ中はここに来ない） */
		if (m_loadPhase == LoadPhase::Done)
		{
			switch (m_gamePhase)
			{
			case GamePhase::CountDown:
				m_uiManager->RenderCountDown(rc);
				break;
			case GamePhase::Playing:
				m_uiManager->RenderPlaying(rc);
				break;
			case GamePhase::Finishing:
				m_uiManager->RenderFinishing(rc);
				break;
			}
		}
	}


	bool InGameScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		/** タイトルへ戻る */
		if (m_goTitle)
		{
			SoundManager::Get().StopAllSE();
			id = TitleScene::ID();
			waitTime = 0.5f;
			return true;
		}
		/** リザルトへ */
		if (m_nextScene)
		{
			SoundManager::Get().StopAllSE();
			id = ResultScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}
}