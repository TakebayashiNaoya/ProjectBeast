/**
 * @file WpWarningAnimStatus.h
 * @brief WpWarningのアニメーションステータス
 */
#pragma once
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Model/UIAnimationStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief WpWarningのアニメーションステータス
		 */
		class WpWarningAnimStatus : public UIAnimationStatus
		{
		public:
			/** JSONから読み込むときの構造体のAnimデータ */
			struct AnimData
			{
				Vector3 startScale;
				Vector3 endScale;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};

		public:
			WpWarningAnimStatus();
			virtual ~WpWarningAnimStatus() override;


		public:
			/** 構造体のゲッター関数 */
			inline AnimData GetGrowAndShrinkAnimationData() const { return m_growAndShrinkAnimData; }


		public:
			void SetUp() override final;
			void Update() override final;


		private:
			/** アイコンの拡大縮小アニメーションのデータ */
			AnimData m_growAndShrinkAnimData;
		};
	}
}


