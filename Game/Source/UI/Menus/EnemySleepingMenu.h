/**
 * @file EnemySleepingMenu.h
 * @brief クマの起床ゲージ表示クラス
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		class EnemySleepingMenu : public MenuBase
		{
		public:
			EnemySleepingMenu();

			void Update() override;
			void InitializeLogic() override;


		public:
			void SetTargetPosition(const Vector3& position) { m_targetPosition = position; }
			void SetSleepingRate(const float rate){ m_sleepingRate = rate; }
			void SetDraw(const bool isDraw) { m_isDraw = isDraw; }


		private:
			Vector3 m_targetPosition;
			float m_sleepingRate;
			bool m_isDraw;
			bool m_isStartAnimationPlayed;
		};
	}
}
