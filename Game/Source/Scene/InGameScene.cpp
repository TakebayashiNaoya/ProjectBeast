/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyControllerManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Util/JsonConverter.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/CameraController.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Util/JsonConverter.h"

#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"

#include <random>


namespace app
{
	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
		delete m_enemyController;
		delete m_enemy;
		delete m_daddyPenguin;

		actor::EnemyControllerManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		actor::EnemyManager::DestroyInstance();
		//for (auto*& p : m_childPenguins) {
		//	delete p;
		//	p = nullptr;
		//}
		actor::ChildPenguinManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();

		app::TimeManager::DestroyInstance();
		app::ScoreManager::DestroyInstance();

		DeleteGO(m_ocean);
	}


	bool InGameScene::Start()
	{
		ScoreManager::CreateInstance();
		TimeManager::CreateInstance();

		app::core::ParameterManager::CreateInstance();
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyManager::CreateInstance();
		m_phase = LoadPhase::Stage;
		m_childIndex = 0;
		return true;
	}

	void InGameScene::Update()
	{
		switch (m_phase)
		{
		case LoadPhase::Stage:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/stage/stageObject.json");
			actor::StageSystem::GetInstance()->CreateStageObject(json);
			m_phase = LoadPhase::Daddy;
			break;
		}

		case LoadPhase::Daddy:
			m_daddyPenguin = new actor::DaddyPenguin();
			m_daddyPenguin->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
			m_daddyPenguin->StartWrapper();
			m_phase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
			// CHILD_PENGUIN_NUM が 100 以上に設定されていることを前提とします
			if (m_childIndex < CHILD_PENGUIN_NUM)
			{
				// 乱数生成器の初期化（staticにすることで毎フレーム初期化されるのを防ぎます）
				static std::random_device rd;
				static std::mt19937 gen(rd());
				// -2000.0f から 2000.0f の範囲の乱数を生成
				static std::uniform_real_distribution<float> dis(-2000.0f, 2000.0f);

				// X, Y, Zをランダムに設定
				Vector3 pos = Vector3(dis(gen), 0.0f, dis(gen));

				actor::ChildPenguinManager::GetInstance()->CreateChildPenguin(1);

				const auto& children = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();
				auto* child = children.back();

				// ----------------------------------------------------
				// ① 先にタイプを設定する（ここでステートマシンが生成される）
				// ----------------------------------------------------
				if (m_childIndex < 50)
				{
					child->SetChildPenguinType(app::actor::EnChildPenguinType::Serious);
				}
				else if (m_childIndex < 100)
				{
					child->SetChildPenguinType(app::actor::EnChildPenguinType::Clingy);
				}

				// ----------------------------------------------------
				// ② その後に座標をセットする（生成されたステートマシンに座標が渡る）
				// ----------------------------------------------------
				child->SetPosition(pos);
				child->GetStateMachine()->SetPosition(pos);
				child->StartWrapper();

				++m_childIndex;
			}
			else {
				auto* manager = app::actor::ChildPenguinManager::GetInstance();
				manager->SetDaddyPenguin(m_daddyPenguin);
				m_phase = LoadPhase::Enemy;
			}
			break;

		case LoadPhase::Enemy:
		{
			// JSONを読み込んでマネージャーに渡すだけで完了
			nlohmann::json enemyJson;
			util::JsonConverter::IsLoadJsonFile(enemyJson, "Assets/parameter/character/enemy/EnemyLayout.json");
			actor::EnemyManager::GetInstance()->LoadEnemies(enemyJson);

			m_phase = LoadPhase::Camera;
			break;
		}

		case LoadPhase::Camera:
		{
			// CameraSteeringの初期化
			camera::CameraSteering::Config config;
			m_cameraSteering.SetConfig(config);
			m_cameraSteering.SetTargetCharacter(m_daddyPenguin);

			// GameCameraを登録してアクティブにする
			auto gameCamera = std::make_shared<camera::GameCamera>();
			camera::CameraManager::Get().Register(camera::GameCamera::ID(), gameCamera);
			camera::CameraManager::Get().SwitchCamera(camera::GameCamera::ID());
			m_phase = LoadPhase::Ocean;
			break;
		}

		case LoadPhase::Ocean:
			m_ocean = NewGO<Ocean>(0);
			m_phase = LoadPhase::Done;
			break;

		case LoadPhase::Done:
		{
			// 通常更新
			actor::StageSystem::GetInstance()->Update();
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();

			actor::ChildPenguinManager::GetInstance()->Update();

			actor::EnemyManager::GetInstance()->Update();

			// CameraSteeringの結果をGameCameraに反映
			auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(camera::GameCamera::ID());
			if (gameCamera) {
				camera::CameraData data = gameCamera->GetCameraData();
				m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
				gameCamera->SetState(data);
			}

			// ノイズのリストをクリア
			NoiseManager::GetInstance().ClearNoises();

			//if (g_pad[0]->IsTrigger(enButtonA))
			//{
			//	nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
			// 
			//	ResultScene::SetResult(
			//		TimeManager::GetInstance().GetCurTime(),
			//		ScoreManager::GetInstance().GetCollectedCount()
			//	);
			//	m_nextScene = true;
			//}
			break;
		}

		default: break;
		}

	}

	void InGameScene::PauseUpdate()
	{}


	void InGameScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);
		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		//for (auto* p : m_childPenguins) {
		//	if (p) p->RenderWrapper(rc);
		//}
		actor::ChildPenguinManager::GetInstance()->Render(rc);

		actor::EnemyManager::GetInstance()->Render(rc);
	}


	bool InGameScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = ResultScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}
}