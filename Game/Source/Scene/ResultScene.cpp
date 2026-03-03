/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "TitleScene.h"


namespace app
{
	ResultScene::ResultScene()
	{
	}


	ResultScene::~ResultScene()
	{
	}


	bool ResultScene::Start()
	{
		m_resultRender.Init("Assets/sprite/Result.DDS", 1920.0f, 1080.0f);
		return true;
	}


	void ResultScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}


	void ResultScene::Render(RenderContext& rc)
	{
		m_resultRender.Draw(rc);
	}


	bool ResultScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = TitleScene::ID();
			waitTime = 3.0f;
			return true;
		}
		return false;
	}
}