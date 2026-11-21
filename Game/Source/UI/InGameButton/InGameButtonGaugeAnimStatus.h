/**
 * @file InGameButtonGaugeAnimStatus.h
 * @brief インゲームボタンの再集合ゲージ専用のアニメーションステータスクラス
 */
#pragma once
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Model/UIAnimationStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲームボタンの再集合ゲージ専用のアニメーションステータスクラス
		 * @note UIAnimationStatusを継承しているため、UIアニメーションを制御するためのステータスを持つことが可能
		 */
		class InGameButtonGaugeAnimStatus : public UIAnimationStatus
		{
		public:
			/** JSONから読み込むときの構造体のAnimデータ */
			struct AnimData
			{
				Vector4 startColor;
				Vector4 endColor;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};


		public:
			InGameButtonGaugeAnimStatus();
			virtual ~InGameButtonGaugeAnimStatus() override;

			/** 構造体のゲッター群 */
			AnimData GetRegroupLockAnimData() const { return m_regroupLockAnimData; }
			AnimData GetRegroupUnlockAnimData() const { return m_regroupUnlockAnimData; }

			void SetUp() override;
			void Update() override;


		private:
			/** これらにJSONの情報を入れていく */
			AnimData m_regroupLockAnimData;
			AnimData m_regroupUnlockAnimData;
		};
	}
}
