#include "stdafx.h"
#include "SceneManager.h"

SceneManager* SceneManager::m_instance = nullptr;

SceneManager::SceneManager()
	:m_currentScene(nullptr)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	if (m_currentScene) {
		/** 現在のシーンのアップデートを呼び出す */
		m_currentScene->Update();
		/** シーンの切り替え要求があるか確認する */
		/** 要求があった場合現在のシーンを削除する */
		if (m_currentScene->RequesutScene(m_nextSceneId)) {
			delete m_currentScene;
			m_currentScene = nullptr;
		}
	}

	/** シーンの切り替えが要求されていたらシーンを生成する */
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
	m_currentScene = it->second.get();
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
	/**
	 * 最初のシーンを設定
	 * タイトルシーンを最初にする場合
	 * SceneManager::GetInstance()->CreateScene(TitleScene::ID());
	 */
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
