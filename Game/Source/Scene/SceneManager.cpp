/**
 * @file SceneManager.cpp
 * @brief シーンの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"

#include "DebugScene.h"
#include "InGameScene.h"
#include "ResultScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

#include "Source/Core/Fade.h"


namespace app
{
	SceneManager* SceneManager::m_instance = nullptr;


	SceneManager::SceneManager()
		:m_currentScene(nullptr)
	{
		// ここでシーン追加
		AddSceneMap<app::DebugScene>();
		AddSceneMap<app::TitleScene>();
		AddSceneMap<app::InGameScene>();
		AddSceneMap<app::ResultScene>();

		// 初期シーン生成
		CreateScene(app::TitleScene::ID());
	}


	SceneManager::~SceneManager()
	{
	}


	void SceneManager::Update()
	{
		if (m_currentScene) {

			if (g_pad[0]->IsTrigger(enButtonSelect))
			{
				m_isPause = !m_isPause;
			}
			if (m_isPause) {
				return;
			}

			m_currentScene->Update();
			if (m_currentScene->RequesutScene(m_nextSceneId, m_waitTime)) {
				delete m_currentScene;
				m_currentScene = nullptr;

				core::Fade::Get().Enable();
			}
		}

		if (m_nextSceneId != INVALID_SCENE_ID) {
			m_elapsedTime += g_gameTime->GetFrameDeltaTime();
			if (m_elapsedTime >= m_waitTime) {
				CreateScene(m_nextSceneId);
				m_waitTime = 0.0f;
				m_elapsedTime = 0.0f;
				m_nextSceneId = INVALID_SCENE_ID;

				core::Fade::Get().Disable();
			}
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
		auto& createSceneFunc = it->second;
		m_currentScene = createSceneFunc();
		m_currentScene->Start();
	}
}