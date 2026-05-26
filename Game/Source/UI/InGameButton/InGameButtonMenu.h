/**
 * @file InGameButtonMenu.h
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


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
			/** ボタンのアイコンの更新処理 */
			void ButtonIconUpdate();

			bool IsInputAButton() const;
			bool IsInputBButton() const;
			bool IsInputXButton() const;
			bool IsInputYButton() const;


		private:
			InGameStartingAnimLogic m_startingAnimLogic;
		};
	}
}