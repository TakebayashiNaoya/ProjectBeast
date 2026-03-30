/**
 * @file InGameTimerMenu.h
 * @brief インゲームタイマーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		enum class EnInGameTimerType : uint8_t
		{
			Frame,
			FrameBackGround,
			Max
		};


		class TimerIcon : public Noncopyable
		{
		public:
			TimerIcon();
			~TimerIcon();
			void Update();
			void SetUIIcon(UIIcon* icon);

			inline void SetIsDraw(bool isDraw)
			{
				if (m_icon)m_icon->m_isDraw = isDraw;
			}


		private:
			EnInGameTimerType m_type;
			UIIcon* m_icon;
		};


		class TimerDigit : public Noncopyable
		{
		public:
			TimerDigit();
			~TimerDigit();
			void Update();
			void SetUIDigit(UIDigit* digit);
			inline void SetIsDraw(bool isDraw)
			{
				if (m_digit)m_digit->m_isDraw = isDraw;
			}


		public:
			void SetValue(int value);


		private:
			UIDigit* m_digit = nullptr;
		};


		class InGameTimerMenu : public MenuBase
		{
			using InGameTimerClass = MenuBase;

		public:
			InGameTimerMenu();
			~InGameTimerMenu();

			void Update()override;

			/** 描画するかどうか */
			void SetIsDraw(bool isDraw);

			/** タイマーの時間を設定 */
			inline void SetTime(float time) { m_currentTime = time; }


		private:
			/** 今の時間 */
			float m_currentTime;

			using Icon = std::unique_ptr<TimerIcon>;
			using Digit = std::unique_ptr<TimerDigit>;
			using Key = uint32_t;

			std::unordered_map<Key, Icon>m_timerIconMap;
			std::unordered_map<Key, Digit>m_timerDigitMap;
		};
	}
}
