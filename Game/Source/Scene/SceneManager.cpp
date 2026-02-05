/**
 * @file SceneManager.cpp
 * @brief シーンの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "SceneManager.h"


namespace app
{
	SceneManager* SceneManager::m_instance = nullptr;


	SceneManager::SceneManager()
		:m_currentScene(nullptr)
	{
		// ここでシーン追加
		AddSceneMap<app::DebugScene>();

		// 初期シーン生成
		CreateScene(app::DebugScene::ID());
	}


	SceneManager::~SceneManager()
	{
	}


	void SceneManager::Update()
	{
		if (m_currentScene) {
			m_currentScene->Update();
			if (m_currentScene->RequesutScene(m_nextSceneId)) {
				delete m_currentScene;
				m_currentScene = nullptr;
			}
		}

		if (m_nextSceneId != INVALID_SCENE_ID) {
			CreateScene(m_nextSceneId);
			m_nextSceneId = INVALID_SCENE_ID;
		}
	}


	void SceneManager::Render(RenderContext& rc)
	{
		if (m_currentScene) {
			m_currentScene->Render(rc);
		}
	}


	void SceneManager::CreateScene(const uint32_t id)
	{
		auto it = m_sceneMap.find(id);
		if (it == m_sceneMap.end()) {
			K2_ASSERT(false, "新規シーンが追加されていません。\n");
		}
		m_currentScene = it->second();
		m_currentScene->Start();
	}
}