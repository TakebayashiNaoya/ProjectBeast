/**
 * @file DaddyPenguinStatus.h
 * @brief 親ペンギンのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 親ペンギンのステータスクラス
		 */
		class DaddyPenguinStatus : public CharacterStatus
		{
		public:
			// ここに親ペンギン固有のステータス用のゲッター関数を追加していく
			/**
			 * @brief 移動速度(スニーク)を取得
			 * @return 移動速度(スニーク)
			 */
			inline float GetSneakSpeed() const { return m_sneakSpeed; }


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


		public:
			DaddyPenguinStatus();
			~DaddyPenguinStatus() override;


		private:
			// ここに親ペンギン固有のステータスを追加していく
			/** 移動速度(スニーク) */
			float m_sneakSpeed;
		};
	}
}

