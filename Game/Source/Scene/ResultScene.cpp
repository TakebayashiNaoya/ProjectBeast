/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "TitleScene.h"


namespace app
{
	ResultScene::ResultScene()
		:m_clearTime(0.0f)
		, m_collectedPenguin(0)
	{}


	ResultScene::~ResultScene()
	{
		app::TimeManager::DestroyInstance();
		app::ScoreManager::DestroyInstance();
	}


	bool ResultScene::Start()
	{
		m_resultRender.Init("Assets/sprite/Result.DDS", 1920.0f, 1080.0f);

		m_clearTime = app::TimeManager::GetInstance().GetCurTime();
		m_collectedPenguin = app::ScoreManager::GetInstance().GetCollectedCount();

		return true;
	}


	void ResultScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}


	void ResultScene::PauseUpdate()
	{}


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