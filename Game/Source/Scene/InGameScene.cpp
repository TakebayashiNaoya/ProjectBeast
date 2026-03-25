/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"

#include "Source/Actor/Stage/IStage.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"


namespace app
{
	InGameScene::InGameScene()
	{}


	InGameScene::~InGameScene()
	{
		delete m_stage;
		delete m_daddyPenguin;
		for (auto*& p : m_childPenguins) {
			delete p;
			p = nullptr;
		}
	}


	bool InGameScene::Start()
	{
		app::core::ParameterManager::CreateInstance();
		m_phase = LoadPhase::Stage;
		m_childIndex = 0;
		return true;
	}

	void InGameScene::Update()
	{
		switch (m_phase)
		{
		case LoadPhase::Stage:
			m_stage = new actor::IStageObject();
			m_stage->Init("Assets/modelData/stage/floor.tkm");
			m_stage->StartWrapper();
			m_phase = LoadPhase::Daddy;
			break;

		case LoadPhase::Daddy:
			m_daddyPenguin = new actor::DaddyPenguin();
			m_daddyPenguin->SetPosition(Vector3::Zero);
			m_daddyPenguin->StartWrapper();
			m_phase = LoadPhase::Children;
			break;

		case LoadPhase::Children:
			if (m_childIndex < CHILD_PENGUIN_NUM)
			{
				Vector3 pos = Vector3(5.0f + (m_childIndex % 10) * 3.0f,
					0.0f,
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

		case LoadPhase::Done:
			// 通常更新
			if (m_stage)        m_stage->UpdateWrapper();
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
		if (m_stage)        m_stage->RenderWrapper(rc);
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