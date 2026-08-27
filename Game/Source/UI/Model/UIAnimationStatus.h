/**
 * @file UIAnimationStatus.h
 * @biref UIAnimationStatusでUIAnimationに必要なステータスの基底クラス
 */
#pragma once
#include "Source/Core/IStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @biref UIStatusの基底クラス
		 */
		class UIAnimationStatus : public core::IStatus
		{
		public:
			UIAnimationStatus() = default;
			virtual ~UIAnimationStatus() = default;

			/**
			 * @brief セットアップUI
			 * @detail ステータスの持ち主が呼び出す。純粋仮想関数。
			 */
			virtual void SetUp() override;
			/**
			 * @brief 更新処理
			 */
			virtual void Update() override;
		};
	}
}


