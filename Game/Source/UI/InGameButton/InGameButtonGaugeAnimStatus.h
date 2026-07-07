/**
 * @file InGameButtonGaugeAnimStatus.h
 * @brief インゲームボタンのスタミナゲージ専用のアニメーションステータスクラス
 * @author 立山
 */
#pragma once
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Model/UIAnimationStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲームボタンのスタミナゲージ専用のアニメーションステータスクラス
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
			AnimData GetJumpLockAnimData() const { return m_jumpLockAnimData; }
			AnimData GetJumpUnlockAnimData() const { return m_jumpUnlockAnimData; }
			AnimData GetSlideLockAnimData() const { return m_slideLockAnimData; }
			AnimData GetSlideUnlockAnimData() const { return m_slideUnlockAnimData; }

			void SetUp() override;
			void Update() override;


		private:
			/** これらにJSONの情報を入れていく */
			AnimData m_jumpLockAnimData;
			AnimData m_jumpUnlockAnimData;
			AnimData m_slideLockAnimData;
			AnimData m_slideUnlockAnimData;
		};
	}
}
