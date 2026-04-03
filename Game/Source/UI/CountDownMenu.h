/**
 * @file CountDownMenu.h
 * @brief カウントダウンの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"
#include <unordered_map>

namespace app
{
	namespace ui
	{
		/** カウントダウンのタイプ */
		enum class EnCountDownType : uint8_t
		{
			GO,
			First,
			Second,
			Third,
			Max,
			Start,
			None,
			Finished
		};


		class CountDownIcon
		{
		public:
			CountDownIcon(EnCountDownType type);
			~CountDownIcon();
			void Update();
			void SetUIIcon(UIIcon* icon);
			inline void SetIsDraw(bool isDraw)
			{
				m_icon->m_isDraw = isDraw;
			}
			inline EnCountDownType GetType()const
			{
				return m_type;
			}


		private:
			EnCountDownType m_type;
			UIIcon* m_icon;

			// 現在のカウントダウンの時間。
			float m_currentTime;
		};


		class CountDownMenu : public MenuBase
		{
			using CountDownClass = MenuBase;


		public:
			CountDownMenu();

			void Update()override;
			void InitializeLogic()override;
			void CalcCount();
			void ResetCountDown();

			EnCountDownType GetCurrentCountType();
			/**
			 * @brief 現在のカウントダウンのタイプを取得する
			 * @return 現在のカウントダウンのタイプ
			 */
			inline bool IsCountDownStart() const
			{
				return m_countDownStartFlag;
			}
			/**
			 * @brief カウントダウンの開始フラグを設定する
			 * @param flag カウントダウンの開始フラグ
			 */
			inline void SetCountDownStartFlag(bool flag)
			{
				m_countDownStartFlag = flag;
			}
			/**
			 * @brief カウントダウンの終了フラグを取得する
			 * @return カウントダウンの終了フラグ
			 */
			inline bool IsCountDownFinished()const
			{
				return m_countDownFinishedFlag;
			}
			/**
			 * @brief カウントダウンの終了フラグを設定する
			 * @param flag カウントダウンの終了フラグ
			 */
			inline void SetCountDownFinishedFlag(bool flag)
			{
				m_countDownFinishedFlag = flag;
			}


			/**
			 * @brief カウントダウンの遅延開始フラグを取得する
			 * @return カウントダウンの遅延開始フラグ
			 */
			inline bool IsDelayStart()const {return m_isDelayStart; }


			/**
			 * @brief カウントダウンの遅延開始フラグを設定する
			 * @param isDelay カウントダウンの遅延開始フラグ
			 * @note trueが開始、falseがディレイ終了
			 */
			inline void SetIsDelay(bool isDelay) 
			{
              m_isDelayStart = isDelay; 
				if (m_isDelayStart)
				{
					m_delayTime = 0.0f;
					m_countDownStartFlag = false;
					for (const auto& icon : m_countDownMap)
					{
						icon.second->SetIsDraw(false);
					}
				}
			}
			

		private:
			float m_time;
			float m_delayTime;

			EnCountDownType m_currentCountType;
			using Icon = std::unique_ptr<CountDownIcon>;
			using Key = uint32_t;
			std::unordered_map<Key, Icon>m_countDownMap;

			bool m_isDelayStart;
			bool m_countDownStartFlag;
			bool m_countDownFinishedFlag;
		};
	}
}
