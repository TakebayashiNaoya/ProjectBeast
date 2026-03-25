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
			void Update()override;
			void InitializeLogic()override;
			void ResetTimer();

			/**
			 * @brief アイコンの初期化用
			 */
			void InitializeIcon();
			/**
			 * @brief ディジットの初期化用
			 */
			void InitializeDigit();

			/**
			 * @brief 今のタイマーが動いているかどうかを返す
			 * @return タイマーが動いているかどうか
			 */
			inline bool IsTimerActive()const { return m_isTimerActive; }


			// タイマーの開始と停止。
			void StartTimer() { m_isTimerActive = true; }
			void StopTimer() { m_isTimerActive = false; }


		private:
			float m_currentTime;
			bool m_isTimerActive;


			using Icon = std::unique_ptr<TimerIcon>;
			using Digit = std::unique_ptr<TimerDigit>;
			using Key = uint32_t;

			std::unordered_map<Key, Icon>m_timerIconMap;
			std::unordered_map<Key, Digit>m_timerDigitMap;
		};
	}
}
