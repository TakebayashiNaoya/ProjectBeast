/**
 * @file AchievementNotificationMenu.h
 * @brief アチーブメント通知画面の動的処理クラス
 * @author 立山
 */
#pragma once
#include "Source/Achivement/AchievementManager.h"
#include "Source/UI/Menu.h"
#include "Source/UI/Model/AchievementAnimStatus.h"


namespace app
{
	namespace achievement
	{
		class AchievementBase;
	}


	namespace ui
	{
		/**
		 * @brief アチーブメント通知画面の動的処理クラス
		 */
		class AchievementNotificationMenu : public MenuBase
		{
		public:
			AchievementNotificationMenu();
			~AchievementNotificationMenu() override;


			/** 更新処理 */
			void Update() override;

			/** 描画処理 */
			void Render(RenderContext& rc) override;


			/** UIのロジック初期化処理 */
			void InitializeLogic() override;


		private:
			struct NotificationData
			{
				app::achievement::AchievementBase* achievement;
			};


			std::queue<NotificationData> m_notificationQueue;

			bool m_isPlaying;

			UIAnimationSequence m_sequence;

			std::unique_ptr<AchievementAnimStatus> m_animStatus;

			std::vector<bool> m_wasAchievedList; // 各アチーブメントの過去の達成状態をメモするリスト
		};

	}
}
