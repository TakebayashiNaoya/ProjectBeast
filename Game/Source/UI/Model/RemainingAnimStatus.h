/**
 * @file RemaingAnimStatus.h
 * @brief 子ペンギンの救助数と総数のアニメーションステータスクラス
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
		 * @brief 子ペンギンの救助数と総数のアニメーションステータスクラス
		 */
		class RemainingAnimStatus : public UIAnimationStatus
		{
		private:
			/** アニメーションデータ */
			struct AnimData
			{
				Vector3 rStartTlanslate;
				Vector3 rEndSink;
				Vector3 start;
				Vector3 end;
				Vector3 start2;
				Vector3 end2;
				Vector3 start3;
				Vector3 end3;
				Vector3 rSinkDown;
				Vector3 rSinkBounce;
				Vector3 tStartTlanslate;
				Vector3 tEndTlanslate;

				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};


			/** バウンドアニメーション */
			AnimData m_startBound;		// 救助数増加アニメーション。
			AnimData m_endBound;		// 救助数増加アニメーション。
			AnimData m_startSink;		// 救助数減少アニメーション。
			AnimData m_endBounceDownUp; // 救助数減少アニメーション。
			AnimData m_bounceDown;		// 総数減少アニメーション。
			AnimData m_bounceUp;		// 総数減少アニメーション。


		public:
			RemainingAnimStatus();
			virtual ~RemainingAnimStatus() override;

			void SetUpUI() override;

			void Update() override;
		};
	}
}
