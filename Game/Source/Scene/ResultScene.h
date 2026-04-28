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


	class ResultScene : public IScene
	{
		appScene(ResultScene);


	public:
		ResultScene();
		~ResultScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate()override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	public:
		// InGameScene から遷移前に呼ぶ
		static void SetResult(float clearTime, int collectedPenguin)
		{
			s_clearTime = clearTime;
			s_collectedPenguin = collectedPenguin;
		}


	private:
		// ★追加：スコア計算とアチーブメントUI構築
		void CalcTotalScore();


	private:
		bool m_nextScene = false;
		float m_clearTime;
		int m_collectedPenguin;
		float m_totalScore;

		std::vector<app::achievement::AchievementBase*> m_allAchievementList;

		static float s_clearTime;
		static int   s_collectedPenguin;

		app::ui::Layout m_layout;
		app::ui::ResultMenu* m_resultMenu;
	};
}