/**
 * @file CPReactionAnimStatus.h
 * @brief CPReactionのアニメーションステータス
 * @author 藤谷
 */
#pragma once
#include "UIAnimationStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		/**
		 * @biref CPReactionのステータス
		 */
		class CPReactionAnimStatus : public UIAnimationStatus
		{
			/** JSONから読み込むときの構造体のAnimデータ */
			struct AnimData
			{
				float startRot;
				float endRot;
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

		public:
			CPReactionAnimStatus();
			virtual ~CPReactionAnimStatus() override;


		public:
			/** 構造体のゲッター関数 */
			inline AnimData GetSwayAnimationData() const { return m_swayAnimationData; }


		public:
			void SetUpUI() override final;
			void Update() override final;


		private:
			AnimData m_swayAnimationData;
		};
	}
}


