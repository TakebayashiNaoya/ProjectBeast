/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"


namespace app
{
	InGameScene::InGameScene()
	{
	}


	InGameScene::~InGameScene()
	{
	}


	bool InGameScene::Start()
	{
		return true;
	}


	void InGameScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}


	void InGameScene::Render(RenderContext& rc)
	{
	}


	bool InGameScene::RequesutScene(uint32_t& id)
	{
		if (m_nextScene) {
			id = ResultScene::ID();
			return true;
		}
		return false;
	}
}