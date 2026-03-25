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


namespace app
{
	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
		actor::StageSystem::DestroyInstance();
		delete m_daddyPenguin;
		for (auto*& p : m_childPenguins) {
			delete p;
			p = nullptr;
		}
	}


	bool InGameScene::Start()
	{
		app::core::ParameterManager::CreateInstance();
		actor::StageSystem::CreateInstance();
		m_phase = LoadPhase::Stage;
		m_childIndex = 0;
		return true;
	}

	void InGameScene::Update()
	{
		switch (m_phase)
		{
		case LoadPhase::Stage:
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, "Assets/parameter/stage/stageObject.json");
			actor::StageSystem::GetInstance()->CreateStageObject(json);
			m_phase = LoadPhase::Daddy;
			break;

		case LoadPhase::Daddy:
			m_daddyPenguin = new actor::DaddyPenguin();
			m_daddyPenguin->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
			m_daddyPenguin->StartWrapper();
			m_phase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
			if (m_childIndex < CHILD_PENGUIN_NUM)
			{
				Vector3 pos = Vector3(10.0f + (m_childIndex % 10) * 3.0f,
					100.0f,
					(m_childIndex / 10) * 3.0f);
				m_childPenguins[m_childIndex] = new actor::ChildPenguin();
				m_childPenguins[m_childIndex]->SetPosition(pos);
				m_childPenguins[m_childIndex]->StartWrapper();
				++m_childIndex;
			}
			else {
				m_phase = LoadPhase::Done;
			}
			break;

		case LoadPhase::Camera:

		case LoadPhase::Done:
			// 通常更新
			actor::StageSystem::GetInstance()->Update();
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			for (auto* p : m_childPenguins) if (p) p->UpdateWrapper();
			if (g_pad[0]->IsTrigger(enButtonA)) m_nextScene = true;
			break;

		default: break;
		}
	}

	void InGameScene::PauseUpdate()
	{}


	void InGameScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);
		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		for (auto* p : m_childPenguins) {
			if (p) p->RenderWrapper(rc);
		}
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