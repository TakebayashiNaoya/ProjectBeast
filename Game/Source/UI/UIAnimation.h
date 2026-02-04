/**
 * @file UIAnimation.h
 * @brief UIAnimationの機能群
 * @author 忽那
 */
#pragma once


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class UIBase;


		/**
		 * @brief UIAnimationの機能
		 */
		class UIAnimationBase
		{
		public:
			UIAnimationBase();
			~UIAnimationBase() = default;


			/** 純粋仮想関数 */
			virtual void Update() = 0;
			virtual void PlayAnimation() = 0;
			virtual bool IsPlayAnimation() = 0;


		public:
			/**
			 * @brief UIの設定
			 * @param ui UIBaseのポインタ
			 */
			void SetUI(UIBase* ui) { m_ui = ui; }


		protected:
			/** UIBaseのポインタ */
			UIBase* m_ui;
		};
	}
}