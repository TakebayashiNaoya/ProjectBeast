/**
 * @file ResultMenu.h
 * @brief リザルト画面の動的処理クラス
 * @author 立山
 */
#pragma once
#include "Source/Achivement/AchievementManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Menu.h"
#include "Source/UI/Parts/UIParts.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief リザルト画面の動的処理クラス
		 */
		class ResultMenu :public MenuBase
		{
		public:
			ResultMenu();
			~ResultMenu();

			void Update() override;
			void InitializeLogic() override;


			// シーンからスコアやクリア時間などのデータを受け取り、初期化する
			void SetResultData(float clearTime, int collectedPenguin, float totalScore,
				const std::vector<app::achievement::AchievementBase*>& achievements);


			// Aボタンでの次シーン遷移が可能な状態か判定する
			bool IsReadyToNextScene() const { return m_titleButtonShown; }


		private:
			void SetupAchievementUI();
			void UpdateRevealSequence();


		private:
			float m_clearTime;
			int m_collectedPenguin;
			float m_totalScore;
			std::vector<app::achievement::AchievementBase*> m_allAchievementList;

			app::SEHandle m_drumRollHandle;

			std::vector<UIIcon*> m_checkIconList; // 達成済みチェックアイコン（表示順）
			UIDigit* m_totalDigit;                // トータルスコア表示用

			// 演出用タイマーとフラグ
			float m_checkRevealTimer;
			int   m_checkRevealIndex;
			bool  m_allChecksRevealed;

			float m_postCheckTimer;
			bool  m_totalScoreShown;
			bool  m_titleButtonShown;
		};
	}
}
