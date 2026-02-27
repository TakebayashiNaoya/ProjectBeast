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
	}


	bool TitleScene::RequesutScene(uint32_t& id)
	{
		if (m_nextScene) {
			id = InGameScene::ID();
			return true;
		}
		return false;
	}
}