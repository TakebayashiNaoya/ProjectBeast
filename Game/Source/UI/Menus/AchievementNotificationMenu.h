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

			bool m_isInitialized;

			std::unique_ptr<AchievementAnimStatus> m_animStatus;

			std::vector<bool> m_wasAchievedList; // 各アチーブメントの過去の達成状態をメモするリスト

			// 各UIの本来の初期位置を保存しておく変数
			Vector3 m_defaultBgPos;
			Vector3 m_defaultCheckPos;
			Vector3 m_defaultStampPos;
			Vector3 m_defaultNameTextPos;

			enum class AnimState
			{
				Idle,           // 0: 待機
				FadeIn,         // 1: フェードイン中
				StampWait,      // 2: スタンプ待機
				StampPlay,      // 3: スタンプ再生中
				FadeOutWait,    // 4: フェードアウト待機
				FadeOut         // 5: フェードアウト中
			};

			AnimState m_animState;
			float m_animTimer;

			/** 最後に確認したリロードバージョン */
			int m_lastReloadVersion = -1;
		};

	}
}
