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

		// ① ステージ生成
		m_stage = new actor::IStageObject();
		m_stage->Init("Assets/modelData/stage/floor.tkm"); // 実際のパスに合わせる
		m_stage->StartWrapper();

		// ② 親ペンギン生成
		m_daddyPenguin = new actor::DaddyPenguin();
		m_daddyPenguin->SetPosition(Vector3::Zero);
		m_daddyPenguin->StartWrapper();

		// ③ 子ペンギン100体生成
		Vector3 pos = Vector3(5.0f, 0.0f, 0.0f);
		for (int i = 0; i < CHILD_PENGUIN_NUM; i++) {
			m_childPenguins[i] = new actor::ChildPenguin();
			m_childPenguins[i]->SetPosition(pos);
			m_childPenguins[i]->StartWrapper();
			// 少しずつずらして配置
			pos.x += 3.0f;
			if ((i + 1) % 10 == 0) {
				pos.x = 5.0f;
				pos.z += 3.0f;
			}
		}

		return true;
	}


	void InGameScene::Update()
	{
		if (m_stage)        m_stage->UpdateWrapper();
		if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
		for (auto* p : m_childPenguins) {
			if (p) p->UpdateWrapper();
		}

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}

	void InGameScene::PauseUpdate()
	{
	}


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
			waitTime = 3.0f;
			return true;
		}
		return false;
	}
}