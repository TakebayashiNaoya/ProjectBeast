/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#include "stdafx.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "Source/Actor/Character/Player/Player.h"
#include "Source/Core/ParameterManager.h"



namespace app
{
	InGameScene::InGameScene()
	{
		core::ParameterManager::CreateInstance();
	}


	InGameScene::~InGameScene()
	{
		delete m_player;
		m_player = nullptr;
	}


	bool InGameScene::Start()
	{
		m_player = new actor::Player();
		m_player->StartWrapper();
		return true;
	}


	void InGameScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
		m_player->UpdateWrapper();
	}


	void InGameScene::Render(RenderContext& rc)
	{
		m_player->RenderWrapper(rc);
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