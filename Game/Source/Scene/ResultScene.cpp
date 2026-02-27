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
	}


	bool ResultScene::RequesutScene(uint32_t& id)
	{
		if (m_nextScene) {
			id = TitleScene::ID();
			return true;
		}
		return false;
	}
}