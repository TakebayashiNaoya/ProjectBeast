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
		{
		}
		
		
		TimerIcon::~TimerIcon()
		{}
		
		
		void TimerIcon::Update()
		{}


		void TimerIcon::SetUIIcon(UIIcon * icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");
		}





		/***********************************************/


		TimerDigit::TimerDigit()
			: m_digit(nullptr)
		{
		}

		TimerDigit::~TimerDigit()
		{
		}


		void TimerDigit::Update()
		{
		}


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
			: m_currentTime(300.0f)
			, m_isTimerActive(false)
		{
		}


		void InGameTimerMenu::Update()
		{
			// タイマーがfalseのときだけ、時間を減らす。
			if (m_isTimerActive)
			{
				float deltaTime = g_gameTime->GetFrameDeltaTime();
				if (m_currentTime > 0.0f)
				{
					m_currentTime -= deltaTime;
					// 0秒以下にならないように制限。
					if (m_currentTime <= 0.0f)
					{
						m_currentTime = 0.0f;
						m_isTimerActive = true;
					}
				}
			}
			
			// floatからintにキャスト。(小数点以下は切り捨て)
			int displayTime = static_cast<int>(m_currentTime);

			K2_LOG("TEST: %d \n", displayTime);
			
			// Mapからディジットを取得して、値をセット。
			//auto it = m_timerDigitMap.find(Hash32("InGameTimerDigit"));
			//if (it != m_timerDigitMap.end())
			//{
			//	it->second->SetValue(displayTime);
			//}

			auto* timerDigit = GetUI<UIDigit>(Hash32("InGameTimerDigit"));
			if (timerDigit) {
				timerDigit->SetNumber(displayTime);
			}



			// MenuBaseの更新処理。
			InGameTimerClass::Update();
		}


		void InGameTimerMenu::InitializeLogic()
		{
			InitializeIcon();
			InitializeDigit();
		}


		void InGameTimerMenu::ResetTimer()
		{
			m_currentTime = 300.0f;
			m_isTimerActive = false;
		}


		void InGameTimerMenu::InitializeIcon()
		{
			//m_timerIconMap.clear();
			//m_timerIconMap.reserve(TIMER_ICON_SIZE);
			//
			//for (const auto& info : TIMER_ICON_KEYS)
			//{
			//	Icon timerIcon = std::make_unique<TimerIcon>();
			//	timerIcon->SetUIIcon(GetUI<UIIcon>(info.key));
			//	m_timerIconMap.emplace(info.key, std::move(timerIcon));
			//}
		}


		void InGameTimerMenu::InitializeDigit()
		{
			//m_timerDigitMap.clear();
			//Digit timerDigit = std::make_unique<TimerDigit>();
			//timerDigit->SetUIDigit(GetUI<UIDigit>(Hash32("InGameTimerDigit")));
			//m_timerDigitMap.emplace(Hash32("InGameTimerDigit"), std::move(timerDigit));
		}
	}
}