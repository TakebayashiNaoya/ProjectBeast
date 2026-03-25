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
#include "Resource/ResourceManager.h"


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
	{}


	void SceneManager::Update()
	{
		float delta = g_gameTime->GetFrameDeltaTime();

		if (m_currentScene) {

			if (g_pad[0]->IsTrigger(enButtonSelect))
			{
				m_isPause = !m_isPause;
			}
			if (!m_isPause) {
				m_currentScene->Update();
			}
			else
			{
				m_currentScene->PauseUpdate();
			}

			switch (m_transitionState)
			{
			case TransitionState::Idle:
				if (m_currentScene)
				{
					if (!m_isPause) {
						m_currentScene->Update();
					}
					if (m_currentScene->RequesutScene(m_nextSceneId, m_fadeDuration))
					{
						core::Fade::Get().FadeOut(m_fadeDuration);
						m_transitionState = TransitionState::FadingOut;
					}
				}
				break;

			case TransitionState::FadingOut:
				// FadeOut 完了待ち（画面が完全に暗くなるまで）
				if (core::Fade::Get().IsFadeOutComplete())
				{
					delete m_currentScene;
					m_currentScene = nullptr;
					CreateScene(m_nextSceneId); // Start() → アクター生成 → 非同期ロード開始
					m_nextSceneId = INVALID_SCENE_ID;
					m_transitionState = TransitionState::LoadingScene;
				}
				break;

			case TransitionState::LoadingScene:
				// 全リソースのロード完了待ち
				if (nsBeastEngine::ResourceManager::GetInstance().IsIdle())
				{
					K2_LOG("LoadComplete");
					core::Fade::Get().FadeIn(m_fadeDuration);
					m_transitionState = TransitionState::FadingIn;
				}
				break;

			case TransitionState::FadingIn:
				// FadeIn 完了待ち（Update は呼ばない → キャラ・タイマーは動かない）
				if (!core::Fade::Get().IsFading())
				{
					m_fadeDuration = 0.0f;
					m_transitionState = TransitionState::Idle;
				}
				break;
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