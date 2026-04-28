/**
 * @file PBWakingUpTimerStatus.h
 * @biref PB起床タイマー専用のステータスクラス
 * @author 忽那
 */
#pragma once
#include "UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @biref PB起床タイマー専用のステータスクラス
		 */
		class PBWakingUpTimerStatus : public UIStatus
		{
		public:
			PBWakingUpTimerStatus();
			~PBWakingUpTimerStatus()override;

			/*
			 * @brief セットアップUI
			 * @note ステータスの持ち主が呼び出す
			 */
			void SetUpUI() override;

			/**
			 * @biref 更新処理
			 */
			void Update() override;

			/** 起床タイマー専用のゲッター */
			float GetTimerFirstValue() const { return m_timerFirstValue; }
			float GetTimerSecondValue()const { return m_timerSecondValue; }
			float GetTimerThirdValue()const { return m_timerThirdValue; }
			float GetTimerFourthValue()const { return m_timerFourthValue; }
			float GetOffsetValueY()const { return m_offsetValueY; }
			float GetOffsetValueX()const { return m_offsetValueX; }

			const Vector4& GetGreenColor()const { return m_greenColor; }
			const Vector4& GetYellowColor()const { return m_yellowColor; }
			const Vector4& GetRedColor()const { return m_redColor; }


		private:
			/** ここだけの変数 */
			float m_timerFirstValue;
			float m_timerSecondValue;
			float m_timerThirdValue;
			float m_timerFourthValue;
			float m_offsetValueY;
			float m_offsetValueX;
			Vector4 m_greenColor;
			Vector4 m_yellowColor;
			Vector4 m_redColor;
		};
	}
}


