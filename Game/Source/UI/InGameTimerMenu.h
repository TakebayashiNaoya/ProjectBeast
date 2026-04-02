/**
 * @file InGameTimerMenu.h
 * @brief インゲームタイマーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"
#include <map>


namespace app
{
	namespace ui
	{
		enum class EnInGameTimerType : uint8_t
		{
			Frame,
			FrameBackGround,
			Max,
		};


		class TimerIcon
		{
		public:
			TimerIcon();
			~TimerIcon();

			void Update();
			void SetUIIcon(UIIcon* icon);
			void SetIsDraw(bool isDraw) { if (m_icon) m_icon->m_isDraw = isDraw; }


		private:
			UIIcon* m_icon;
		};


		class TimerDigit
		{
		public:
			TimerDigit();
			~TimerDigit();

			void Update();
			void SetUIDigit(UIDigit* digit);
			void SetValue(int value);
			void SetIsDraw(bool isDraw) { if (m_digit) m_digit->m_isDraw = isDraw; }


		private:
			UIDigit* m_digit;
		};


		class InGameTimerMenu : public MenuBase
		{
			using InGameTimerClass = MenuBase;

		public:
			InGameTimerMenu();
			~InGameTimerMenu() = default;

			void Update() override;
			void InitializeLogic() override;

			/**
			 * @brief 現在のタイムを設定する
			 * @param time 現在のタイム（秒）
			 */
			inline void SetTime(const float time) { m_currentTime = time; }

			void SetIsDraw(bool isDraw);


		private:
			/** 現在のタイム（BattleManagerから通知されてセットされる） */
			float m_currentTime = 0.0f;

			std::map<uint32_t, std::unique_ptr<TimerIcon>>  m_timerIconMap;
			std::map<uint32_t, std::unique_ptr<TimerDigit>> m_timerDigitMap;
		};
	}
}