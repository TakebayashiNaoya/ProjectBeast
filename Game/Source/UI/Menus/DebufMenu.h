/**
 * @file DebufMenu.h
 * @brief 甘えん坊ペンギンから親ペンギンにデバフを掛ける演出のメニュー
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 甘えん坊ペンギンから親ペンギンにデバフを書ける演出メニュー
		 */
		class DebufMenu : public MenuBase
		{
		public:
			DebufMenu();
			~DebufMenu() override;

			void Update() override;
			void Render(RenderContext& rc) override;
			void InitializeLogic() override;


		public:
			/**
			 * @brief 描画の設定
			 * @param isDraw 描画するかのフラグ
			 */
			void SetDraw(const bool isDraw) { m_isDraw = isDraw; }


		private:
			/** アニメーションロジック */
			InGameStartingAnimLogic m_startingAnimLogic;
			/** 描画 */
			bool m_isDraw;
		};
	}
}
