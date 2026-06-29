/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/UI/Layout.h"


namespace app
{
	namespace ui
	{
		class ResultMenu;
	}


	/**
	 * @brief リザルトシーンを管理するクラス
	 */
	class ResultScene : public IScene
	{
		appScene(ResultScene);

	public:
		ResultScene();
		~ResultScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;

		// インゲームシーンから遷移前に呼ぶ
		static void SetResult(float clearTime, int collectedPenguin)
		{
			s_collectedPenguin = collectedPenguin;
		}


	private:
		// スコア計算とアチーブメントUI構築
		void CalcTotalScore();


	private:
		bool m_nextScene = false;
		float m_clearTime;
		int m_collectedPenguin;
		float m_totalScore;

		std::vector<app::achievement::AchievementBase*> m_allAchievementList;
		static int   s_collectedPenguin;

		app::ui::Layout m_layout;
		app::ui::ResultMenu* m_resultMenu;
	};
}