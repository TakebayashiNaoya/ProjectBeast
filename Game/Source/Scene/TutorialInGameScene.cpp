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
	namespace
	{
		/**
		 * チュートリアルは実績一覧を常時表示するため、
		 * 画面下部のインゲームボタン等と重ならないよう少し上にずらす。
		 */
		constexpr float TUTORIAL_ACHIEVEMENT_OFFSET_Y = 40.0f;
	}

	void TutorialInGameScene::OnLoadComplete()
	{
		m_tutorialController.Initialize(m_daddyPenguin);

		InGameUIManager::GetInstance()->SetAchievementPositionOffsetY(TUTORIAL_ACHIEVEMENT_OFFSET_Y);
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
