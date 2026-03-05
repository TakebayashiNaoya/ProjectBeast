/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "TitleScene.h"


namespace app
{
	TitleScene::TitleScene()
	{
	}


	TitleScene::~TitleScene()
	{
	}


	bool TitleScene::Start()
	{
		m_titleRender.Init("Assets/sprite/Title.DDS", 1920.0f, 1080.0f);
		return true;
	}


	void TitleScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}


	void TitleScene::Render(RenderContext& rc)
	{
		m_titleRender.Draw(rc);
	}


	bool TitleScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = InGameScene::ID();
			waitTime = 3.0f;
			return true;
		}
		return false;
	}
}