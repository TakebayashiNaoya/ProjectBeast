/**
 * @file InGameButtonMenu.h
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲーム中にボタンメニューを表示するMenuクラス
		 * @details ゲーム中にボタンの操作方法を表示するUIを管理するクラスです。
		 */
		class InGameButtonMenu : public MenuBase
		{
		public:
			InGameButtonMenu();
			~InGameButtonMenu() override = default;
			void Update() override;
			void InitializeLogic() override;


		private:
			/** ゲーム開始時のアニメーションを更新する */
			void UpdateGameStartingAnimation();
			/** ボタンのアイコンの更新処理 */
			void ButtonIconUpdate();

			bool IsInputAButton() const;
			bool IsInputBButton() const;
			bool IsInputXButton() const;
			bool IsInputYButton() const;


		private:
			// ゲーム開始時のアニメーションフラグ

			/** 再生し始めたかどうか */
			bool m_isStartedGameStartingAnimation;
			/** 再生中かどうか */
			bool m_isPlayingGameStartingAnimation;
			/** 終了したかどうか */
			bool m_isFinishedGameStartingAnimation;
		};
	}
}