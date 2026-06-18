/**
 * @file SceneManager.cpp
 * @brief シーンの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"

#include "DebugScene.h"
#include "EasyInGameScene.h"
#include "NormalInGameScene.h"
#include "TutorialInGameScene.h"
#include "ResultScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

#include "Resource/ResourceManager.h"
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
		AddSceneMap<app::EasyInGameScene>();
		AddSceneMap<app::NormalInGameScene>();
		AddSceneMap<app::TutorialInGameScene>();
		AddSceneMap<app::ResultScene>();

		// 初期シーン生成
		CreateScene(app::TitleScene::ID());
	}


	SceneManager::~SceneManager()
	{}


	void SceneManager::Update()
	{
		//if (!m_currentScene) return;

		// ポーズ切り替え
		if (g_pad[0]->IsTrigger(enButtonSelect))
		{
			m_isPause = !m_isPause;
		}

		switch (m_transitionState)
		{
		case TransitionState::Idle:
			// 通常のゲームプレイ：シーンを更新
			if (!m_isPause) {
				m_currentScene->Update();
			}
			else {
				m_currentScene->PauseUpdate();
			}
			// シーン遷移要求を判定
			if (m_currentScene->RequesutScene(m_nextSceneId, m_fadeDuration))
			{
				core::Fade::Get().FadeOut(m_fadeDuration);
				m_transitionState = TransitionState::FadingOut;
			}
			break;

		case TransitionState::FadingOut:
			if (core::Fade::Get().IsFadeOutComplete()) {
				delete m_currentScene;
				m_currentScene = nullptr;
				core::Fade::Get().ShowLoadingCircle();
				m_transitionState = TransitionState::LoadingScene; // ここではCreateしない
			}
			break;

		case TransitionState::LoadingScene:
			if (!m_currentScene && m_nextSceneId != INVALID_SCENE_ID) {
				CreateScene(m_nextSceneId);
				m_nextSceneId = INVALID_SCENE_ID;
			}
			if (m_currentScene) m_currentScene->Update();

			// リソース完了かつシーンの段階ロード完了を待つ
			if (nsBeastEngine::ResourceManager::GetInstance().IsIdle() &&
				m_currentScene->IsLoaded())
			{
				// ローディングサークルをフェードイン開始前に非表示にする
				// FadingIn 中は m_state == FadeIn のため背景は描画され続け黒画面にならない
				core::Fade::Get().HideLoadingCircle();
				core::Fade::Get().FadeIn(m_fadeDuration);
				m_transitionState = TransitionState::FadingIn;
			}
			break;

		case TransitionState::FadingIn:
			// FadeIn 完了待ち
			// シーン更新を行い、初期化済みモデルを表示できるようにする
			m_currentScene->Update();
			if (!core::Fade::Get().IsFading())
			{
				m_fadeDuration = 0.0f;
				m_transitionState = TransitionState::Idle;
			}
			break;
		}
	}


	void SceneManager::Render(RenderContext& rc)
	{
		if (m_currentScene) {
			if (m_transitionState == TransitionState::LoadingScene)
			{
				return;
			}
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