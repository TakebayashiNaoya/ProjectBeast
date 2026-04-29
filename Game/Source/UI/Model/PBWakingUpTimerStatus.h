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
			~PBWakingUpTimerStatus() override;

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
			float GetRatioProgress()const { return m_ratioProgress; }
			float GetDegreeValue()const { return m_degreeValue; }
			float GetDegreeMaxValue()const { return m_degreeMaxValue; }
			float GetInitialPosZ()const { return m_initialPosZ; }
			float GetResetValue()const { return m_resetValue; }
			float GetOffsetPosY()const { return m_offsetPosY; }

			const Vector2& GetArrowPivot()const { return m_arrowPivot; }
			const Vector4& GetSkeltonColor()const { return m_skeltonColor; }


		private:
			/** ここだけの変数 */
			float m_timerFirstValue;
			float m_timerSecondValue;
			float m_timerThirdValue;
			float m_timerFourthValue;
			float m_offsetValueY;
			float m_degreeValue;
			float m_ratioProgress;
			float m_degreeMaxValue;
			float m_initialPosZ;
			float m_resetValue;
			float m_offsetPosY;
			Vector2 m_arrowPivot;
			Vector4 m_skeltonColor;
		};
	}
}


