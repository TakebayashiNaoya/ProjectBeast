/**
 * @file ResultMenu.h
 * @brief リザルト画面の動的処理クラス
 * @author 立山
 */
#pragma once
#include "ScorePopupAnimatorMenu.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Menu.h"
#include "Source/UI/Parts/UIParts.h"


namespace app
{
	namespace ui
	{
		/** @brief result.json の dynamic_layout セクションに対応する構造体 */
		struct ResultDynamicLayout
		{
			Vector3 achieveStartPos = { -200.0f, 150.0f, 0.0f };
			float   achieveOffsetY = -80.0f;
			float   achieveOffsetXCheck = -400.0f;
			float   achieveOffsetXName = 60.0f;
			float   achieveOffsetXBack = 60.0f;
			float   achieveNameFontSize = 0.5f;
			float   achieveNamePivotX = 0.0f;
			float   achieveBackW = 780.0f;
			float   achieveBackH = 120.0f;
			float   achieveBoxW = 60.0f;
			float   achieveBoxH = 60.0f;
			float   achieveCheckW = 60.0f;
			float   achieveCheckH = 60.0f;

		};


		/**
		 * @brief リザルト画面の動的処理クラス
		 */
		class ResultMenu : public MenuBase
		{
		public:
			ResultMenu();
			~ResultMenu();

			void Update() override;
			void Render(RenderContext& rc);
			void InitializeLogic() override;


			/**
			 * @brief シーンからスコアなどのデータを受け取り、初期化する
			 * @param collectedPenguin 収集したペンギンの数
			 * @param totalScore 合計スコア
			 * @param achievements アチーブメントリスト
			 * @param scorePerAchieve アチーブメント1件あたりのスコア
			 */
			void SetResultData(int collectedPenguin, float totalScore,
				const std::vector<app::achievement::AchievementBase*>& achievements,
				float scorePerAchieve);


			// Aボタンでの次シーン遷移が可能な状態か判定する
			bool IsReadyToNextScene() const { return m_titleButtonShown; }


		private:
			/** @brief アチーブメントUIの構築処理 */
			void SetupAchievementUI();
			/** @brief 演出シーケンスの更新処理 */
			void UpdateRevealSequence();
			/** @brief 動的UIの構築処理 */
			void BuildDynamicUI();
			/** @brief 動的要素のクリア処理 */
			void ClearDynamicElements();
			/** @brief 動的レイアウトの読み込み処理 */
			void LoadDynamicLayout();


		private:
			int m_collectedPenguin;
			float m_totalScore;
			std::vector<app::achievement::AchievementBase*> m_allAchievementList;

			app::SEHandle m_drumRollHandle;

			bool m_dataSet = false;               // SetResultData が呼ばれたか
			ResultDynamicLayout m_dynLayout;      // JSON から読み込んだレイアウト設定
			std::vector<UIIcon*> m_checkIconList; // 達成済みチェックアイコン（表示順）

			// 演出用タイマーとフラグ
			float m_checkRevealTimer;
			int   m_checkRevealIndex;
			bool  m_allChecksRevealed;

			float m_postCheckTimer;
			float m_countUpTimer;                 // カウントアップ用タイマー
			bool  m_totalScoreShown;
			bool  m_titleButtonShown;

			float m_scorePerAchieve = 0.0f;
			std::vector<std::unique_ptr<ScorePopupAnimator>> m_scorePopups;
		};
	}
}