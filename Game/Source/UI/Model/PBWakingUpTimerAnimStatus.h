/**
 * @file PBWakingUpTimerAnimStatus.h
 * @brief PB起床タイマー専用のステータスクラス
 * @author 忽那
 */
#pragma once
#include "UIAnimationStatus.h"
#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		/**
		 * @biref PB起床タイマー専用のステータスクラス
		 * @note UIAnimationStatusを継承しているため、UIアニメーションを制御するためのステータスを持つことが可能
		 */
		class PBWakingUpTimerAnimStatus : public UIAnimationStatus
		{
		private:
			/** JSONから読み込むときの構造体のAnimデータ */
			struct AnimData
			{
				Vector4 startColor;
				Vector4 endColor;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};


			/** これらにJSONの情報を入れていく */
			AnimData m_firstAnimData;
			AnimData m_secondAnimData;
			AnimData m_thirdAnimData;


		public:
			PBWakingUpTimerAnimStatus();
			virtual ~PBWakingUpTimerAnimStatus()override;


			/** 構造体のゲッター群 */
			AnimData GetFirstAnimData()const { return m_firstAnimData; }
			AnimData GetSecondAnimData()const { return m_secondAnimData; }
			AnimData GetThirdAnimData()const { return m_thirdAnimData; }


			void SetUpUI()override;
			void Update()override;
		};
	}
}


