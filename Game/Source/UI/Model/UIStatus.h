/**
 * @file UIStatus.h
 * @biref UIStatusの基底クラス
 * @author 忽那
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
		class UIStatus : public core::IStatus
		{
		public:
			UIStatus() = default;
			virtual ~UIStatus() = default;

			/**
			 * @brief セットアップUI
			 * @detail ステータスの持ち主が呼び出す。
			 */
			virtual void SetUp() override;
			/**
			 * @brief 更新処理
			 */
			virtual void Update() override;
		};
	}
}


