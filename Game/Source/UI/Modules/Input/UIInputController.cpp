/**
 * @file UIInputController.cpp
 * @brief UI入力制御を行うクラス
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

			// トリガー入力（十字キー）はスティックの状態に関係なく毎回一発判定する。
			// ニュートラル判定より先に見ないと、スティックを触っていないとき
			// （＝十字キーだけで操作しているとき）に下の早期リターンで握り潰される
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

			// ニュートラル域に戻ったら状態をリセットする。
			if (!isPositive && !isNegative)
			{
				m_isNeutral = true;
				m_repeatTimer = 0.0f;
				return Direction::None;
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