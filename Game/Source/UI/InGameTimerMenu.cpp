/**
 * @file InGameTimer.cpp
 * @brief インゲームタイマーの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "InGameTimerMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct TimerInfo
			{
				uint32_t key;
				EnInGameTimerType type;
			};

			// 要素数。
			constexpr int TIMER_ICON_SIZE = static_cast<int>(EnInGameTimerType::Max);
			// 配列。
			constexpr TimerInfo TIMER_ICON_KEYS[TIMER_ICON_SIZE] =
			{
					{ Hash32("InGameTimerFrameIcon"),EnInGameTimerType::Frame }
				,	{ Hash32("InGameTimerFrameBackGroundIcon"),EnInGameTimerType::FrameBackGround }
			};
		}


		TimerIcon::TimerIcon()
			: m_icon(nullptr)
		{}


		TimerIcon::~TimerIcon()
		{}


		void TimerIcon::Update()
		{}


		void TimerIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");
		}





		/***********************************************/


		TimerDigit::TimerDigit()
			: m_digit(nullptr)
		{}

		TimerDigit::~TimerDigit()
		{}


		void TimerDigit::Update()
		{}


		void TimerDigit::SetUIDigit(UIDigit* digit)
		{
			m_digit = digit;
			K2_ASSERT(m_digit != nullptr, "登録失敗です。");
		}


		void TimerDigit::SetValue(int value)
		{
			if (m_digit != nullptr)
			{
				m_digit->SetNumber(value);
			}
		}





		/************************************************/


		InGameTimerMenu::InGameTimerMenu()
			: m_currentTime(0.0f)
		{
			BattleManager::GetInstance().Register(this);
		}


		InGameTimerMenu::~InGameTimerMenu()
		{
			BattleManager::GetInstance().Unregister(this);
		}


		void InGameTimerMenu::Update()
		{
			// floatからintにキャスト。(小数点以下は切り捨て)
			int displayTime = static_cast<int>(m_currentTime);

			auto* timerDigit = GetUI<UIDigit>(Hash32("InGameTimerDigit"));
			if (timerDigit) {
				timerDigit->SetNumber(displayTime);
			}

			// MenuBaseの更新処理。
			InGameTimerClass::Update();
		}


		void InGameTimerMenu::SetIsDraw(bool isDraw)
		{
			for (const auto& icon : m_timerIconMap)
			{
				icon.second->SetIsDraw(isDraw);
			}
			for (const auto& digit : m_timerDigitMap)
			{
				digit.second->SetIsDraw(isDraw);
			}
		}
	}
}