/**
 * @file RemainingChildMenu.h
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		class RemainingChildMenu : public MenuBase
		{
		public:
			RemainingChildMenu();

			void Update() override;
			void InitializeLogic() override;


		public:
			void SetChildNum(const int num) { m_childNum = num; }
			void SetTotalNum(const int num) { m_totalNum = num; }


		private:
			/** 集めたペンギン数 */
			int m_childNum;
			/** ステージ上の総ペンギン数 */
			int m_totalNum;
		};
	}
}