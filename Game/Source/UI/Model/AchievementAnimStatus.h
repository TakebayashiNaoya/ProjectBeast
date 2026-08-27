/**
 * @file AchievementAnimStatus.h
 * @brief アチーブメントアニメーション専用のステータスクラス
 */
#pragma once
#include "UIAnimationStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief アチーブメントアニメーション専用のステータスクラス
		 */
		class AchievementAnimStatus : public UIAnimationStatus
		{
		public:
			/** JSONから読み込むときの構造体のAnimデータ */
			struct AnimData
			{
				Vector2 startPos;
				Vector2 endPos;
				Vector3 startScale; // 拡縮はVector3のことが多いです
				Vector3 endScale;
				Vector4 startColor; // 透明度の操作用
				Vector4 endColor;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};


			AchievementAnimStatus();
			~AchievementAnimStatus();


			/** 構造体のゲッター関数 */
			AnimData GetFadeInData() const { return m_fadeInData; }
			AnimData GetFadeOutData() const { return m_fadeOutData; }
			AnimData GetStampData() const { return m_stampData; }


			void SetUp() override;
			void Update() override;


		private:
			AnimData m_fadeInData;
			AnimData m_fadeOutData;
			AnimData m_stampData;
		};
	}
}


