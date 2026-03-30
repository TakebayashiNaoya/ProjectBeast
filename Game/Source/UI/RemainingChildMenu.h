/**
 * @file RemainingChildMenu.h
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		class RemainingChildMenu : public MenuBase
		{
		public:
			RemainingChildMenu();

			void Update()override;
			void InitializeLogic()override;
			

		public:
			void SetChildNum(const int num) { m_childNum = num; }

		private:
			int m_childNum = 0;
		};
	}
}
