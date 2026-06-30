/**
 * @file TutorialInGameScene.cpp
 * @brief チュートリアルステージ
 * @author 竹林
 */
#include "stdafx.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Manager/InGameUIManager.h"
#include "TutorialInGameScene.h"


namespace app
{
	void TutorialInGameScene::OnLoadComplete()
	{
		m_tutorialController.Initialize(m_daddyPenguin);
	}


	void TutorialInGameScene::OnUpdatePlaying()
	{
		m_tutorialController.Update();
	}


	void TutorialInGameScene::OnRenderPlaying(RenderContext& rc)
	{
		m_tutorialController.Render(rc);

		/** チュートリアルはポーズを開かずにアチーブメント一覧を確認できるようにする */
		InGameUIManager::GetInstance()->RenderAchievementInPlaying(rc);
	}


	bool TutorialInGameScene::OnPauseUpdate()
	{
		return m_tutorialController.PauseUpdate();
	}


	bool TutorialInGameScene::OnPauseRender(RenderContext& rc)
	{
		return m_tutorialController.PauseRender(rc);
	}
}
