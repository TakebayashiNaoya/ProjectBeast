/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "../../../BeastEngine/Resource/ResourceManager.h"
#include "../../../BeastEngine/Resource/ModelResource.h"
#include "Source/Actor/Character/Player/Player.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	DebugScene::DebugScene()
	{}


	DebugScene::~DebugScene()
	{
		for (auto& player : m_players) {
			delete player;
			player = nullptr;
		}
	}


	bool DebugScene::Start()
	{
		app::core::ParameterManager::CreateInstance();
		m_spawnedCount = 0;
		return true;
	}


	void DebugScene::Update()
	{
		for (auto* p : m_players) {
			if (p) {
				p->UpdateWrapper();
			}
		}

		if (m_spawnedCount >= 20) {
			// すでに最大数生成している場合はここで終了
			return;
		}
		// 1体目を生成して非同期ロード開始。
		m_players[m_spawnedCount] = new app::actor::Player;
		m_position.z += 50.0f; /** 少しずつ右にずらして配置 */
		m_position.x += 50.0f;
		m_players[m_spawnedCount]->SetPosition(m_position);
		m_players[m_spawnedCount]->StartWrapper();
		++m_spawnedCount;
	}


	void DebugScene::Render(RenderContext& rc)
	{
		for (auto* p : m_players) {
			if (p) {
				p->RenderWrapper(rc);
			}
		}
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}