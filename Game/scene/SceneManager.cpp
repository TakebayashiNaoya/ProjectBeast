#include "stdafx.h"
#include "SceneManager.h"

SceneManager* SceneManager::m_instance = nullptr;

SceneManager::SceneManager()
{
	//シーンマップにシーンを登録
	//AddSceneMap<TitleScene>(); と書くとTitleSceneが登録される
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	if (m_currentScene) {
		m_currentScene->Update();
		if (m_currentScene->RequesutScene(m_nextSceneId, m_waitTime)) {
			delete m_currentScene;
			m_currentScene = nullptr;

			//
		}
	}

	if (m_nextSceneId != INVALID_SCENE_ID) {
		m_elapsedTime += g_gameTime->GetFrameDeltaTime();
		if (m_elapsedTime >= m_waitTime) {
			CreateScene(m_nextSceneId);
			m_waitTime = 0.0f;
			m_elapsedTime = 0.0f;
			m_nextSceneId = INVALID_SCENE_ID;

			//
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



/***********************************************/


SceneManagerObject::SceneManagerObject()
{
	SceneManager::CreateInstance();
}

SceneManagerObject::~SceneManagerObject()
{
	SceneManager::DestroyInstance();
}

bool SceneManagerObject::Start()
{
	//最初のシーンを設定
	// タイトルシーンを最初にする場合
	//SceneManager::GetInstance()->CreateScene(TitleScene::ID());
	//と書く
	return true;
}

void SceneManagerObject::Update()
{
	SceneManager::GetInstance()->Update();
}

void SceneManagerObject::Render(RenderContext& rc)
{
	SceneManager::GetInstance()->Render(rc);
}
