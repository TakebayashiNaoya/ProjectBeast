/**
 * @file CPReactionAnimStatus.h
 * @brief CPReactionのアニメーションステータス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Model/UIAnimationStatus.h"

#include "Source/Util/Curve.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief CPReactionのステータス
		 */
		class CPReactionAnimStatus : public UIAnimationStatus
		{
		public:
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
			inline const AnimData& GetSwayAnimationData() const { return m_swayAnimationData; }


		public:
			void SetUp() override final;
			void Update() override final;


		private:
			AnimData m_swayAnimationData;
		};
	}
}


