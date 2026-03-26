/**
 * @file InGameScene.h
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
#include "Source/Util/JsonConverter.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/CameraController.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"


namespace app
{
	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
		actor::StageSystem::DestroyInstance();
		delete m_daddyPenguin;
		//for (auto*& p : m_childPenguins) {
		//	delete p;
		//	p = nullptr;
		//}
		actor::ChildPenguinManager::DestroyInstance();
		DeleteGO(m_ocean);
	}


	bool InGameScene::Start()
	{
		app::core::ParameterManager::CreateInstance();
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
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
			if (m_childIndex < CHILD_PENGUIN_NUM)
			{
				// 10列×10行のグリッド状に配置
				const float spacing = 100.0f;
				Vector3 pos = Vector3(
					(m_childIndex % 10) * spacing,   // X: 0～9列
					0.0f,
					(m_childIndex / 10) * spacing);   // Z: 0～9行

				actor::ChildPenguinManager::GetInstance()->CreateChildPenguin(1);

				const auto& children = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();
				auto* child = children.back();
				child->SetDaddyPenguin(m_daddyPenguin);
				child->SetPosition(pos);
				child->StartWrapper();

				++m_childIndex;
			}
			else {
				m_phase = LoadPhase::Camera;
			}
			break;

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

		case LoadPhase::Done:
		{
			// 通常更新
			actor::StageSystem::GetInstance()->Update();
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			//for (auto* p : m_childPenguins) if (p) p->UpdateWrapper();


			actor::ChildPenguinManager::GetInstance()->Update();

			// CameraSteeringの結果をGameCameraに反映
			auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(camera::GameCamera::ID());
			if (gameCamera) {
				camera::CameraData data = gameCamera->GetCameraData();
				m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
				gameCamera->SetState(data);
			}

			if (g_pad[0]->IsTrigger(enButtonA)) m_nextScene = true;
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