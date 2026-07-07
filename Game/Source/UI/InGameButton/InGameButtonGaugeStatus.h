/**
 * @file InGameButtonGaugeStatus.h
 * @brief インゲームボタンのスタミナゲージ専用のステータスクラス
 * @author
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲームボタンのスタミナゲージ専用のステータスクラス
		 */
		class InGameButtonGaugeStatus : public UIStatus
		{
		public:
			InGameButtonGaugeStatus();
			~InGameButtonGaugeStatus() override;

			/*
			 * @brief セットアップUI
			 * @note ステータスの持ち主が呼び出す
			 */
			void SetUp() override;

			/**
			 * @brief 更新処理
			 */
			void Update() override;

			/** スタミナゲージ専用のゲッター */
			float GetJumpFollowSpeed() const { return m_jumpFollowSpeed; }
			float GetSlideFollowSpeed() const { return m_slideFollowSpeed; }


		private:
			/** ここだけの変数 */
			/** ジャンプゲージの表示追従速度（1秒あたりに追従できる割合） */
			float m_jumpFollowSpeed;
			/** スライドゲージの表示追従速度（1秒あたりに追従できる割合） */
			float m_slideFollowSpeed;
		};
	}
}
