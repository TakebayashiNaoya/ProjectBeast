/**
 * @file EnemySleepingMenu.h
 * @brief クマの起床ゲージ表示クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		class EnemySleepingMenu : public MenuBase
		{
		public:
			EnemySleepingMenu();

			void Update()override;
			void InitializeLogic()override;

			/**
			 * @brief 寝ている割合に応じてゲージの色を変える。
			 */
			void VisualColor();


		public:
			void SetTargetPosition(const Vector3& position) { m_targetPosition = position; }
			void SetSleepingRate(const float rate){ m_sleepingRate = rate; }
			void SetDraw(const bool isDraw) { m_isDraw = isDraw; }


		private:
			Vector3 m_targetPosition = Vector3::Zero;
			float m_sleepingRate = 0.0f;
			bool m_isDraw = false;
			bool m_isStartAnimationPlayed = false;
		};
	}
}
