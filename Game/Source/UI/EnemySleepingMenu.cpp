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
			// 50%時にゲージの色を変更するための値。
			constexpr float GAUGE_RATIO_FIFTY_PERCENT = 0.5f;
			// 50%の割合。
			constexpr float RATIO_FIFTY_PERCENT = 0.5f;

			// 25%時にゲージの色を変更するための値。
			constexpr float GAUGE_RATIO_TWENTY_FIVE_PERCENT = 0.25f;
			// 25%の割合。
			constexpr float RATIO_TWENTY_FIVE_PERCENT = 0.25f;

			// 黄色。
			const Vector4 RATIO_COLOR_FIFTY_PERCENT = Vector4(1.0f, 1.0f, 0.5f, 1.0f);
			// オレンジ色。
			const Vector4 RATIO_COLOR_TWENTY_FIVE_PERCENT = Vector4(1.0f, 0.5f, 0.3f, 1.0f);
			// 赤色。
			//const Vector4 RATIO_COLOR_TWENTY_FIVE_PERCENT = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
			// ゲージのリセット色。
			const Vector4 RESET_COLOR = Vector4(0.3f, 0.5f, 0.8f, 1.0f);
			
			constexpr float OFFSET_X = -200.0f;
			constexpr float OFFSET_Y = 150.0f;

			constexpr float RATE_MIN = 0.01f;
		}


		EnemySleepingMenu::EnemySleepingMenu()
		{}


		void EnemySleepingMenu::Update()
		{
			// 描画フラグがfalseなら位置計算もスキップして非表示にする
			// （起動時に原点座標で一瞬表示されるバグを防ぐ）
			if (!m_isDraw)
			{
				auto* gauge = GetUI<UIGauge>(Hash32("GaugeA"));
				if (gauge) gauge->m_isDraw = false;

				auto* iconA = GetUI<UIIcon>(Hash32("SleepIconA"));
				if (iconA) iconA->m_isDraw = false;

				auto* iconB = GetUI<UIIcon>(Hash32("SleepIconB"));
				if (iconB) iconB->m_isDraw = false;

				auto* iconC = GetUI<UIIcon>(Hash32("SleepIconC"));
				if (iconC) iconC->m_isDraw = false;

				MenuBase::Update();
				return;
			}

			// クマの位置によって変える
			Vector2 screenPosition;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPosition, m_targetPosition);

			auto* gauge = GetUI<UIGauge>(Hash32("GaugeA"));
			if (gauge) {
				gauge->m_transform.m_localTransform.m_position.Set(screenPosition.x + OFFSET_X, screenPosition.y + OFFSET_Y, 0.0f);
				gauge->m_transform.m_localTransform.m_scale = Vector3(m_sleepingRate, 1.0f, 1.0f);
				gauge->m_isDraw = m_sleepingRate > RATE_MIN;

				// ゲージの色を変える処理。
				if (m_sleepingRate <= RATIO_FIFTY_PERCENT)
				{
					// 50%以下ならオレンジ色。
					gauge->m_color = RATIO_COLOR_FIFTY_PERCENT;
					if (m_sleepingRate <= RATIO_TWENTY_FIVE_PERCENT)
					{
						// 25%以下なら赤色。
						gauge->m_color = RATIO_COLOR_TWENTY_FIVE_PERCENT;
					}
				}
				else
				{
					// それ以外は通常色。
					gauge->m_color = RESET_COLOR;
				}

			}
			// アイコン
			{
				// 1個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconA"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x - 50.0f + OFFSET_X, screenPosition.y + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_sleepingRate > RATE_MIN;
					}
				}
				// 2個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconB"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x - 30.0f + OFFSET_X, screenPosition.y + 10.0f + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_sleepingRate > RATE_MIN;
					}
				}
				// 3個目
				{
					auto* icon = GetUI<UIIcon>(Hash32("SleepIconC"));
					if (icon) {
						icon->m_transform.m_localTransform.m_position.Set(screenPosition.x + -15.0f + OFFSET_X, screenPosition.y + 20.0f + OFFSET_Y, 0.0f);
						icon->m_isDraw = m_sleepingRate > RATE_MIN;
					}
				}
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void EnemySleepingMenu::InitializeLogic()
		{
			// 生成直後は全て非表示にする（m_isDraw=trueがデフォルトのため）
			auto* gauge = GetUI<UIGauge>(Hash32("GaugeA"));
			if (gauge) gauge->m_isDraw = false;

			auto* iconA = GetUI<UIIcon>(Hash32("SleepIconA"));
			if (iconA) iconA->m_isDraw = false;

			auto* iconB = GetUI<UIIcon>(Hash32("SleepIconB"));
			if (iconB) iconB->m_isDraw = false;

			auto* iconC = GetUI<UIIcon>(Hash32("SleepIconC"));
			if (iconC) iconC->m_isDraw = false;
		}
	}
}