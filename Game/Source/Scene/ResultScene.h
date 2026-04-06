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
		void SetupAchievementUI();


	private:
		void UpdateRevealSequence();


	private:
		bool m_nextScene = false;


	private:
		float m_clearTime;
		int m_collectedPenguin;
		float m_totalScore;

		std::vector<app::achievement::AchievementBase*> m_allAchievementList;


	private:
		std::vector<app::ui::UIIcon*> m_checkIconList; // 達成済みチェックアイコン（表示順）
		float m_checkRevealTimer = 0.0f;   // 経過タイマー
		float m_checkRevealDelay = 1.0f;   // シーン開始から最初の表示までの待機秒数
		float m_checkRevealInterval = 0.5f;   // チェックマーク1つごとの表示間隔（秒）
		int   m_checkRevealIndex = 0;      // 次に表示するインデックス


	private:
		bool  m_allChecksRevealed = false;            // 全チェック表示完了フラグ
		float m_postCheckTimer = 0.0f;             // 全チェック完了後のタイマー
		float m_totalRevealDelay = 0.5f;             // 全チェック完了→スコア表示までの待機秒数
		bool  m_totalScoreShown = false;            // トータルスコア表示済みフラグ
		float m_titleButtonDelay = 0.5f;             // スコア表示→Aボタンガイド表示までの待機秒数
		bool  m_titleButtonShown = false;            // Aボタンガイド表示済みフラグ


	private:
		static float s_clearTime;
		static int   s_collectedPenguin;


	private:
		SpriteRender m_resultRender;
		SpriteRender m_rescueRender;
		SpriteRender m_clearTimeRender;
		SpriteRender m_frame;
		SpriteRender m_totalFrame;
		SpriteRender m_titleBackRender;


	private:
		app::ui::Layout m_layout;


	private:
		app::ui::UIDigit* m_totalDigit = nullptr;
	};
}