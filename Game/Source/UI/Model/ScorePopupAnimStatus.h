/**
 * @file ScorePopupAnimStatus.h
 * @brief スコア加算ポップアップ専用のアニメーションステータスクラス
 */
#pragma once
#include "UIAnimationStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief スコア加算ポップアップ（"+2000"等）専用のアニメーションステータスクラス
		 * @details
		 *   AchievementAnimStatusと同じ構成。コンストラクタでJSONを
		 *   UIAnimationParameterへ読み込ませる役割を持つ。これにより
		 *   UIAnimationFactory::Attach<UIColorAnimation>(...) が
		 *   上のanimKeyを解決できるようになる。
		 *   生の値が個別に必要な場合用にゲッターも用意している。
		 */
		class ScorePopupAnimStatus : public UIAnimationStatus
		{
		public:
			/** 色(フェードイン/フェードアウト)用のキーフレーム */
			struct ColorAnimData
			{
				Vector4 startColor;
				Vector4 endColor;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};


			ScorePopupAnimStatus();
			~ScorePopupAnimStatus();


			/** 構造体のゲッター関数 */
			ColorAnimData GetFadeInData()  const { return m_fadeInData; }
			ColorAnimData GetFadeOutData() const { return m_fadeOutData; }


			void SetUp() override;
			void Update() override;


		private:
			ColorAnimData m_fadeInData;
			ColorAnimData m_fadeOutData;
		};
	}
}
