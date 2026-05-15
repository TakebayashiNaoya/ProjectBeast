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
				Vector3 startBound;
				Vector3 endBound;
				float duration;
				util::EasingType easingType;
				util::LoopMode loopMode;
			};

			/** バウンドアニメーション */
			AnimData m_startBound;
			AnimData m_endBound;
			AnimData m_startSink;
			AnimData m_endSink;
			AnimData m_littleUp;
			AnimData m_littleDown;


		public:
			RemainingAnimStatus();
			virtual ~RemainingAnimStatus() override;

			void SetUpUI() override;

			void Update() override;

			/** ゲッター群 */
			AnimData GetStartBoundData() const { return m_startBound; }

			AnimData GetEndBoundData() const { return m_endBound; }

			AnimData GetStartSinkData() const { return m_startSink; }

			AnimData GetEndSinkData() const { return m_endSink; }

			AnimData GetLittleUpData() const { return m_littleUp; }

			AnimData GetLittleDownData() const { return m_littleDown; }
		};
	}
}
