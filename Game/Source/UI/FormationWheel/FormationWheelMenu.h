/**
 * @file FormationWheelMenu.h
 * @brief 陣形切り替え(LB/RB)とウルト発動可否(LT/RT)をアイコンで表示するクラス
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 陣形/ウルトのボタン操作を示すMenuクラス
		 * @details
		 *   現在の陣形アイコンを中央に大きく表示し、その左右に
		 *   LB/RBで切り替わる陣形アイコンを並べる。
		 *   LT/RTアイコンはウルトが発動可能な間のみ通常色、それ以外はグレーアウトする。
		 */
		class FormationWheelMenu : public MenuBase
		{
		public:
			FormationWheelMenu();
			~FormationWheelMenu() override = default;
			void Update() override;
			void InitializeLogic() override;


		private:
			/**
			 * @brief 現在/前/次の陣形アイコンの表示切り替え
			 */
			void UpdateFormationIcons();

			/**
			 * @brief ウルト発動可否に応じてLT/RTアイコンの色を切り替える
			 */
			void UpdateUltIconColor();
		};
	}
}
