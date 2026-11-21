/**
 * @file InGameButtonGaugeStatus.h
 * @brief インゲームボタンの再集合ゲージ専用のステータスクラス
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲームボタンの再集合ゲージ専用のステータスクラス
		 */
		class InGameButtonGaugeStatus : public UIStatus
		{
		public:
			InGameButtonGaugeStatus();
			~InGameButtonGaugeStatus() override;

			/**
			 * @brief セットアップUI
			 * @note ステータスの持ち主が呼び出す
			 */
			void SetUp() override;

			/**
			 * @brief 更新処理
			 */
			void Update() override;

			/** 再集合ゲージの表示追従速度を取得する */
			float GetRegroupFollowSpeed() const { return m_regroupFollowSpeed; }


		private:
			/** ここだけの変数 */
			/** 再集合ゲージの表示追従速度（1秒あたりに追従できる割合） */
			float m_regroupFollowSpeed;
		};
	}
}
