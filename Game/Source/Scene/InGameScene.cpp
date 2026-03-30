/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"

#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Util/JsonConverter.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/CameraController.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinStateMachine.h"
#include "Source/Noise/NoiseManager.h"

#include "Source/Manager/BattleManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"

#include "Source/UI/Layout.h"
#include "Source/UI/CountDownMenu.h"
#include "Source/UI/InGameTimerMenu.h"
#include "Source/UI/FinishMenu.h"

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

		// アクター
		actor::StageSystem::DestroyInstance();
		delete m_daddyPenguin;
		actor::EnemyManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();
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
			"Assets/parameter/inGameTimer/InGameTimer.json"
		);
		m_timerMenu = m_timerLayout->GetMenu<ui::InGameTimerMenu>();

		m_finishLayout = new ui::Layout();
		m_finishLayout->Initialize<ui::FinishMenu>(
			"Assets/parameter/UI/FinishMenu.json"
		);
		m_finishMenu = m_finishLayout->GetMenu<ui::FinishMenu>();

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
			BattleManager::GetInstance().SetGameActive(false);

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
				BattleManager::GetInstance().SetGameActive(true);

				// タイマー開始
				if (m_timerMenu)
				{
					m_timerMenu->StartTimer();
				}
			}
			break;
		}

		//------------------------------------------------------------
		// プレイ中
		//------------------------------------------------------------
		case GamePhase::Playing:
		{
			// プレイヤー・子ペンギン・シロクマ の更新
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			actor::ChildPenguinManager::GetInstance()->Update();
			actor::EnemyManager::GetInstance()->Update();

			// タイマー UI 更新
			if (m_timerLayout) m_timerLayout->Update();

			// ノイズリストをクリア
			NoiseManager::GetInstance().ClearNoises();

			// 終了判定
			if (CheckGameEnd())
			{
				// ゲームアクティブを落とす
				BattleManager::GetInstance().SetGameActive(false);
				// タイマー停止
				if (m_timerMenu) m_timerMenu->StopTimer();
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


	bool InGameScene::CheckGameEnd()
	{
		auto& score = ScoreManager::GetInstance();
		const int collected = score.GetCollectedCount();
		const int total = score.GetTotalCount();

		//--- クリア条件 ---
		// ① 50匹以上収集
		if (collected >= CLEAR_COUNT)
		{
			BattleManager::GetInstance().SetClear(true);
			return true;
		}
		// ② ステージ上の全ペンギンを収集
		if (total > 0 && collected == total)
		{
			BattleManager::GetInstance().SetClear(true);
			return true;
		}

		//--- ゲームオーバー条件 ---
		// ③ 時間切れ かつ 50匹未満
		if (m_timerMenu && m_timerMenu->IsTimeUp() && collected < CLEAR_COUNT)
		{
			BattleManager::GetInstance().SetClear(false);
			return true;
		}
		// ④ ステージ上の総数が50匹未満（シロクマに食べられた）
		if (total < CLEAR_COUNT)
		{
			BattleManager::GetInstance().SetClear(false);
			return true;
		}

		return false;
	}


	void InGameScene::PauseUpdate()
	{}


	void InGameScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);

		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
		actor::EnemyManager::GetInstance()->Render(rc);

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
		if (m_nextScene)
		{
			id = ResultScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}
}