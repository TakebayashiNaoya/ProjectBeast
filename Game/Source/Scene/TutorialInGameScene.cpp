/**
 * @file TutorialInGameScene.cpp
 * @brief チュートリアルステージ
 * @author 竹林
 */
#include "stdafx.h"
#include "TutorialInGameScene.h"
#include "Source/Scene/SceneManager.h"


namespace app
{
	void TutorialInGameScene::OnLoadComplete()
	{
		// ウィンドウ 1 ─────────────────────────────────────────────────────────
		// clipPath : Assets/Video/Tutorial/Window1/ に PNG フレームを配置する
		// desc     : TutorialWindowDesc の asset を説明画像パスに差し替える
		m_windowLayouts[0].Initialize<ui::TutorialWindowMenu>(
			"Assets/parameter/Tutorial/TutorialWindow1.json");

		// ウィンドウ 2 ─────────────────────────────────────────────────────────
		// JSON を複製して clipPath / asset を変えるだけで増やせる
		m_windowLayouts[1].Initialize<ui::TutorialWindowMenu>(
			"Assets/parameter/Tutorial/TutorialWindow2.json");
	}


	void TutorialInGameScene::OnUpdatePlaying()
	{
		// Playing フェーズ最初のフレームに 1 枚目のウィンドウを開いてポーズ
		if (!m_allWindowsDone && m_currentWindowIndex < 0)
		{
			m_currentWindowIndex = 0;
			m_isTutorialWindowPause = true;

			auto* menu = m_windowLayouts[0].GetMenu<ui::TutorialWindowMenu>();
			if (menu) menu->Open();

			// ゲームをポーズ（以後は OnPauseUpdate が呼ばれる）
			SceneManager::GetInstance()->SetPause(true);
		}
	}


	bool TutorialInGameScene::OnPauseUpdate()
	{
		if (!m_isTutorialWindowPause) return false;

		// 現在のウィンドウを更新（アニメーション・入力）
		m_windowLayouts[m_currentWindowIndex].Update();

		// Closing アニメーションが完了したら次へ進む
		auto* menu = m_windowLayouts[m_currentWindowIndex].GetMenu<ui::TutorialWindowMenu>();
		if (menu && menu->IsClosedByUser())
		{
			m_currentWindowIndex++;

			if (m_currentWindowIndex < WINDOW_COUNT)
			{
				// 次のウィンドウを開く
				auto* nextMenu = m_windowLayouts[m_currentWindowIndex].GetMenu<ui::TutorialWindowMenu>();
				if (nextMenu) nextMenu->Open();
			}
			else
			{
				// 全ウィンドウ完了 → ポーズ解除して通常プレイへ
				m_allWindowsDone = true;
				m_isTutorialWindowPause = false;
				SceneManager::GetInstance()->SetPause(false);
			}
		}

		return true;
	}


	bool TutorialInGameScene::OnPauseRender(RenderContext& rc)
	{
		if (!m_isTutorialWindowPause) return false;

		if (m_currentWindowIndex >= 0 && m_currentWindowIndex < WINDOW_COUNT)
		{
			m_windowLayouts[m_currentWindowIndex].Render(rc);
		}

		return true;
	}
}
