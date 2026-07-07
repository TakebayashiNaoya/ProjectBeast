/**
 * @file UIInputController.cpp
 * @brief UI入力制御を行うクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "UIInputController.h"


namespace app
{
	namespace ui
	{
		Direction AxisInputDetector::Update(float stickValue, bool triggerNegative, bool triggerPositive, float threshold, float repeatInterval)
		{
			const bool isPositive = stickValue > threshold;
			const bool isNegative = stickValue < -threshold;

			// ニュートラル域に戻ったら状態をリセットする。
			if (!isPositive && !isNegative)
			{
				m_isNeutral = true;
				m_repeatTimer = 0.0f;
				return Direction::None;
			}

			// トリガー入力は倒しっぱなし状態に関係なく毎回一発判定する。
			if (triggerPositive)
			{
				m_isNeutral = false;
				m_repeatTimer = repeatInterval;
				return Direction::Positive;
			}
			if (triggerNegative)
			{
				m_isNeutral = false;
				m_repeatTimer = repeatInterval;
				return Direction::Negative;
			}

			// ニュートラルから倒した瞬間は即座に反応する。
			if (m_isNeutral)
			{
				m_isNeutral = false;
				m_repeatTimer = repeatInterval;
				return isPositive ? Direction::Positive : Direction::Negative;
			}

			// 倒しっぱなし中：repeatIntervalが指定されていればオートリピートする。
			if (repeatInterval > 0.0f)
			{
				m_repeatTimer -= g_gameTime->GetFrameDeltaTime();
				if (m_repeatTimer <= 0.0f)
				{
					m_repeatTimer = repeatInterval;
					return isPositive ? Direction::Positive : Direction::Negative;
				}
			}

			return Direction::None;
		}




		AxisInputDetector::AxisInputDetector()
			: m_isNeutral(true)
			, m_repeatTimer(0.0f)
		{}
	}
}