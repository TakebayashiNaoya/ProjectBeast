/**
 * @file EnemySleepingMenu.cpp
 * @brief クマの起床ゲージ表示クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "EnemySleepingMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr float OFFSET_X = -200.0f;
			constexpr float OFFSET_Y = 150.0f;

			constexpr float RATE_MIN = 0.01f;
		}


		EnemySleepingMenu::EnemySleepingMenu()
		{}


		void EnemySleepingMenu::Update()
		{
			// クマの位置によって変える
			Vector2 screenPosition;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPosition, m_targetPosition);

			auto* gauge = GetUI<UIGauge>(Hash32("GaugeA"));
			if (gauge) {
				gauge->m_transform.m_localTransform.m_position.Set(screenPosition.x + OFFSET_X, screenPosition.y + OFFSET_Y, 0.0f);
				gauge->m_transform.m_localTransform.m_scale = Vector3(m_sleepingRate, 1.0f, 1.0f);
				gauge->m_isDraw = m_isDraw && m_sleepingRate > RATE_MIN;
			}
			// アイコン
			{
				// 1個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconA"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x - 50.0f + OFFSET_X, screenPosition.y + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_isDraw && m_sleepingRate > RATE_MIN;
					}
				}
				// 2個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconB"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x - 30.0f + OFFSET_X, screenPosition.y + 10.0f + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_isDraw && m_sleepingRate > RATE_MIN;
					}
				}
				// 3個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconC"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x + -15.0f + OFFSET_X, screenPosition.y + 20.0f + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_isDraw && m_sleepingRate > RATE_MIN;
					}
				}
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void EnemySleepingMenu::InitializeLogic()
		{
			// TODO: 減った時のアニメーションなど追加したい
		}
	}
}

