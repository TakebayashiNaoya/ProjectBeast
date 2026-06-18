/**
 * @file InGameSceneBase.cpp
 * @brief インゲームシーン基底クラス
 * @author 立山、竹林
 */
#include "stdafx.h"
#include "InGameSceneBase.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Graphics/PBRStatus.h"

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
#include "Source/Manager/IglooManager.h"
#include "Source/Manager/InGameUIManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"

#include "Source/Achivement/AchievementManager.h"

#include "Source/UI/Menus/CountDownMenu.h"
#include "Source/UI/Menus/FinishMenu.h"
#include "Source/UI/Menus/PauseScreenMenu.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"

#include "Graphics/Camera/SubCameraManager.h"
#include "Source/Scene/SceneManager.h"
#include "TitleScene.h"

#include "Source/Nature/Ocean.h"
#include "Source/Nature/WhirlpoolManager.h"


namespace app
{
	InGameSceneBase::InGameSceneBase()
	{}


	InGameSceneBase::~InGameSceneBase()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
		/** UIManager（デストラクタでBattleManagerのfunctionもリセットされる） */
		InGameUIManager::DestroyInstance();

		/** アクター */
		delete m_daddyPenguin;
		actor::EnemyManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		nature::WhirlpoolManager::DestroyInstance();
		actor::IglooManager::DestroyInstance();

		/** PBRステータス */
		graphics::PBRStatus::DestroyInstance();

		DeleteGO(m_skyCube);
		nature::Ocean::DestroyInstance();

		/** マネージャー */
		BattleManager::DestroyInstance();
		ScoreManager::DestroyInstance();
		TimeManager::DestroyInstance();

		/** シーン破棄時にサブカメラを強制停止する（タイトル遷移後に残らないよう） */
		nsBeastEngine::SubCameraManager::Get().ForceEnd();

		/** タイトルへ戻る場合は ResultScene を経由しないため、ここで破棄する。
		 *  リザルト遷移の場合は ResultScene::~ResultScene が担当する。 */
		if (m_goTitle) {
			if (app::achievement::AchievementManager::GetInstance()) {
				app::achievement::AchievementManager::DestroyInstance();
			}
		}

		/** 2周目以降のために GameCamera の登録を解除する。
		 *  次の LoadPhase::Camera で新しいインスタンスを Register できるようにする。 */
		camera::CameraManager::Get().Unregister(camera::GameCamera::ID());
	}


	bool InGameSceneBase::Start()
	{
		/** マネージャー生成 */
		app::core::ParameterManager::CreateInstance();
		BattleManager::CreateInstance();
		ScoreManager::CreateInstance();
		TimeManager::CreateInstance();

		/** ステージ固有の制限時間を設定する */
		TimeManager::GetInstance().SetMaxTime(GetTimeLimit());
		TimeManager::GetInstance().ResetTime();

		app::achievement::AchievementManager::CreateInstance();
		app::achievement::AchievementManager::GetInstance()->Start(GetAchievementJsonPath());

		GameLogManager::CreateInstance();

		/** PBRStatus生成 */
		graphics::PBRStatus::CreateInstance();

		/** アクター系シングルトン生成 */
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyManager::CreateInstance();
		actor::IglooManager::CreateInstance();

		/** UIManager生成（Layoutの生成はDaddyPenguin生成後のInitializeで行う） */
		InGameUIManager::CreateInstance();

		/** ロードフェーズ開始 */
		m_loadPhase = LoadPhase::Stage;
		m_childIndex = 0;

		return true;
	}


	void InGameSceneBase::Update()
	{
		//------------------------------------------------------------
		// ロードフェーズ
		//------------------------------------------------------------
		switch (m_loadPhase)
		{
		case LoadPhase::Stage:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, GetStageJsonPath());
			actor::StageSystem::GetInstance()->CreateStageObject(json);
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
			m_daddyPenguin->SetPosition(GetDaddySpawnPos());
			m_daddyPenguin->StartWrapper();

			// DaddyPenguinをディザリングのプレイヤーターゲットとして登録する
			// DaddyPenguinのモデルはデプス描画の対象になり、遮蔽対象リストには含まれない
			nsBeastEngine::OcclusionDitherManager::Get().SetPlayerTarget(
				&m_daddyPenguin->GetModelRender()
			);

			// DaddyPenguin生成後にUIManagerを初期化（睡眠クマ探索のキャプチャに使用）
			InGameUIManager::GetInstance()->Initialize(m_daddyPenguin);

			m_loadPhase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
		{
			/** ステージ固有の生成設定を取得して一括生成 */
			const PenguinSpawnConfig cfg = GetPenguinConfig();
			actor::ChildPenguinManager::GetInstance()->CreateChildPenguins(
				cfg.serious,
				cfg.clingy,
				cfg.naughty,
				cfg.clumsy,
				cfg.caring,
				cfg.spawnRadius
			);

			auto* manager = actor::ChildPenguinManager::GetInstance();
			manager->SetDaddyPenguin(m_daddyPenguin);

			/** ステージ上の総ペンギン数をセット */
			const int totalNum = cfg.serious + cfg.clingy + cfg.naughty + cfg.clumsy + cfg.caring;
			ScoreManager::GetInstance().SetTotalCount(totalNum);

			m_loadPhase = LoadPhase::Enemy;
			break;
		}

		case LoadPhase::Enemy:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, GetEnemyJsonPath());
			actor::EnemyManager::GetInstance()->LoadEnemies(json);

			/** エネミー1体につき探索UIを1つ生成 */
			for (auto* enemy : actor::EnemyManager::GetInstance()->GetEnemies())
			{
				InGameUIManager::GetInstance()->AddSearchLayout(enemy);
			}

			m_loadPhase = LoadPhase::Camera;
			break;
		}

		case LoadPhase::Camera:
		{
			camera::CameraSteering::Config config;
			m_cameraSteering.SetConfig(config);
			m_cameraSteering.SetTargetCharacter(m_daddyPenguin);

			/** 2周目以降は Unregister 済みのため、毎回新しいインスタンスを登録できる */
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
			m_skyCube->SetType(enSkyCubeType_Clear);
			m_skyCube->SetScale(Vector3(700.0f, 600.0f, 700.0f));
			m_skyCube->SetLuminance(0.8f);

			nature::Ocean::CreateInstance();
			nature::Ocean::GetInstance()->Start(GetOceanParameterBinaryPath());

			nature::WhirlpoolManager::CreateInstance();
			nature::WhirlpoolManager::GetInstance()->Start(
				GetWhirlpoolPositionsJsonPath(),
				GetWhirlpoolParameterBinaryPath()
			);

			m_loadPhase = LoadPhase::Done;

			/** ロード完了 → カウントダウン開始 */
			if (InGameUIManager::GetInstance()->GetCountDownMenu())
			{
				InGameUIManager::GetInstance()->GetCountDownMenu()->SetIsDelay(true);
				SoundManager::Get().PlayBGM(enSoundKind_InGame);
			}

			OnLoadComplete();
			break;

		case LoadPhase::Done:
			/** ポーズ終了後の初回フレームでポーズ入場フラグとサブビューをリセットする
			 *  通常ポーズ解除（IsRetry）と異なりチュートリアルポーズはここでリセットされる */
			if (m_isPauseEntered)
			{
				nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(false);
				m_isPauseEntered = false;
			}
			/** ゲームフェーズへ移譲 */
			UpdateGamePhase();
			break;

		default:
			break;
		}
	}


	void InGameSceneBase::UpdateGamePhase()
	{
		/** カメラは常に更新 */
		auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
			camera::GameCamera::ID()
		);
		if (gameCamera)
		{
			camera::CameraData data = gameCamera->GetCameraData();
			m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(data);
		}

		/** ディザリングマネージャーは毎フレーム更新して、カメラとプレイヤーの位置を反映させる */
		OcclusionDitherManager::Get().SetPlayerTarget(&m_daddyPenguin->GetModelRender());
		OcclusionDitherManager::Get().Update();

		/** ステージは常に更新 */
		actor::StageSystem::GetInstance()->Update();
		if (nature::Ocean::GetInstance()) {
			nature::Ocean::GetInstance()->Update();
		}
		nature::WhirlpoolManager::GetInstance()->Update();

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
			auto* uiMngr = InGameUIManager::GetInstance();

			uiMngr->UpdateCountDown();

			/** カウントダウン中のSE制御 */
			auto* countDownMenu = uiMngr->GetCountDownMenu();
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

				// カウントダウン終了 → プレイ中のUIに切り替える。
				InGameUIManager::GetInstance()->UpdatePlaying();
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
			auto* uiMngr = InGameUIManager::GetInstance();
			uiMngr->UpdatePlaying();

			BattleManager::GetInstance().Update();
			TimeManager::GetInstance().Update();

			app::achievement::AchievementManager::GetInstance()->Update();

			/** ログ毎秒ティック */
			m_logTickTimer += g_gameTime->GetFrameDeltaTime();
			if (m_logTickTimer >= 0.1f)
			{
				m_logTickTimer -= 0.1f;
				if (auto* lm = GameLogManager::GetInstance())
					lm->RecordTick(m_daddyPenguin);
			}

			/** ノイズリストをクリア */
			NoiseManager::GetInstance().ClearNoises();

			OnUpdatePlaying();

			/** 終了判定 */
			if (BattleManager::GetInstance().GetBattleState() == BattleManager::EnBattleState::Finished)
			{
				SoundManager::Get().StopAllSE();

				/** FINISH 演出開始 */
				auto* finishMenu = uiMngr->GetFinishMenu();
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
			auto* uiMngr = InGameUIManager::GetInstance();
			uiMngr->UpdateFinishing();

			/** ホイッスルは演出開始時に1回だけ鳴らす */
			if (!m_isWhistlePlayed)
			{
				SoundManager::Get().PlaySE(enSoundKind_Whistle, false);
				m_isWhistlePlayed = true;
			}

			/** 演出終了 → リザルトへ */
			auto* finishMenu = uiMngr->GetFinishMenu();
			if (finishMenu && finishMenu->IsFinished())
			{
				SoundManager::Get().StopBGM();
				m_nextScene = true;

				const float clearTime = TimeManager::GetInstance().GetCurTime();
				const int   rescued = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
				ResultScene::SetResult(clearTime, rescued);

				/** アチーブメント最終判定（FinalCondition 型を評価） */
				float logScore = 0.0f;
				if (auto* am = app::achievement::AchievementManager::GetInstance())
				{
					am->FinalizeAchievements();

					// ResultScene::CalcTotalScore() と同じ式でスコアを算出してログに記録する
					int achievedCount = 0;
					for (auto* achieve : am->GetAllAchievements())
					{
						if (achieve && achieve->IsAchieved()) achievedCount++;
					}
					constexpr int   MIN_ACHIEVE_MULTIPLIER = 1;
					constexpr float BASE_TIME_MULTIPLIER = 1.0f;
					constexpr float SCORE_TIME_DIVISOR = 100.0f;
					constexpr float SCORE_BASE_MULTIPLIER = 100.0f;

					const int   achieveMultiplier = (achievedCount > 0) ? achievedCount : MIN_ACHIEVE_MULTIPLIER;
					const float timeMultiplier = BASE_TIME_MULTIPLIER + (clearTime / SCORE_TIME_DIVISOR);
					logScore = static_cast<float>(rescued) * SCORE_BASE_MULTIPLIER
						* static_cast<float>(achieveMultiplier) * timeMultiplier;
				}

				/** ログをファイルへ書き出し */
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->Flush(GetStageName(), clearTime, rescued, logScore);
					GameLogManager::DestroyInstance();
				}
			}
			break;
		}
		}
	}


	void InGameSceneBase::PauseUpdate()
	{
		/** ポーズ開始フレームに1回だけ全SEを停止し、サブビューを非表示にする
		 *  チュートリアルポーズを含むすべてのポーズ種別に適用するため
		 *  OnPauseUpdate() の前に実行する */
		if (!m_isPauseEntered)
		{
			SoundManager::Get().StopAllSE();
			nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(true);
			m_isPauseEntered = true;
		}

		/** ポーズ中もアチーブメントJSONのホットリロードとUI反映を行う */
		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			am->CheckHotReload();
		}
		if (auto* uiMngr = InGameUIManager::GetInstance())
		{
			uiMngr->UpdateAchievementHotReload();
		}

		/** 派生クラスが独自ポーズを処理する場合は通常ポーズをスキップ */
		if (OnPauseUpdate()) return;

		switch (m_pauseState)
		{
			//------------------------------------------------------------
			// ポーズメニュー
			//------------------------------------------------------------
		case PauseState::Pause:
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* pauseMenu = uiMngr->GetPauseMenu();
			if (!pauseMenu) break;

			uiMngr->UpdatePause();

			/** ゲームに戻る */
			if (pauseMenu->IsRetry())
			{
				pauseMenu->IsRetry(false);
				nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(false);
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
				auto* tutorialMenu = uiMngr->GetTutorialMenu();
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
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* soundOptionMenu = uiMngr->GetSoundOptionMenu();
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
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* tutorialMenu = uiMngr->GetTutorialMenu();
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


	void InGameSceneBase::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);

		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
		actor::EnemyManager::GetInstance()->Render(rc);

		/** ポーズ中の描画 */
		if (SceneManager::GetInstance()->IsPause())
		{
			if (!OnPauseRender(rc))
			{
				/** 通常ポーズ画面の描画（派生クラスが処理しない場合） */
				auto* uiMngr = InGameUIManager::GetInstance();
				switch (m_pauseState)
				{
				case PauseState::Pause:
					uiMngr->RenderPause(rc);
					break;
				case PauseState::SoundOption:
					uiMngr->RenderSoundOption(rc);
					break;
				case PauseState::Tutorial:
					uiMngr->RenderTutorial(rc);
					break;
				}
			}
			return;
		}

		/** UI 描画（ポーズ中はここに来ない） */
		if (m_loadPhase == LoadPhase::Done)
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			switch (m_gamePhase)
			{
			case GamePhase::CountDown:
				uiMngr->RenderCountDown(rc);
				break;
			case GamePhase::Playing:
				uiMngr->RenderPlaying(rc);
				OnRenderPlaying(rc);
				break;
			case GamePhase::Finishing:
				uiMngr->RenderFinishing(rc);
				break;
			}
		}
	}


	bool InGameSceneBase::RequesutScene(uint32_t& id, float& waitTime)
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
