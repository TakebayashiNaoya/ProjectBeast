/**
 * @file UIStatus.h
 * @biref UIStatusの基底クラス
 * @author 忽那
 */
#pragma once


namespace app
{
	namespace ui
	{
		/**
		 * @biref UIStatusの基底クラス
		 */
		class UIStatus
		{
		public:
			UIStatus() = default;
			virtual ~UIStatus() = default;

			/**
			 * @brief セットアップUI
			 * @detail ステータスの持ち主が呼び出す。純粋仮想関数。
			 */
			virtual void SetUpUI() = 0;
			/**
			 * @brief 更新処理
			 */
			virtual void Update() = 0;
		};
	}
}


