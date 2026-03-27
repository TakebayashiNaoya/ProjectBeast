/**
 * @file PenguinStatus.h
 * @brief ペンギンのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ペンギンのステータスクラス
		 */
		class PenguinStatus : public CharacterStatus
		{
		public:
			/**
			 * @brief 移動速度(スニーク)を取得
			 * @return 移動速度(スニーク)
			 */
			inline float GetSneakSpeed() const { return m_sneakSpeed; }
			/**
			 * @brief 移動速度(スライド)を取得
			 * @return 移動速度(スライド)
			 */
			inline float GetSlideSpeed() const { return m_slideSpeed; }
			/**
			 * @brief ジャンプパワーを取得
			 * @return ジャンプパワー
			 */
			inline float GetJumpPower() const { return m_jumpPower; }


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			virtual void Setup() override;


			virtual void Update() override;


		public:
			PenguinStatus();
			virtual ~PenguinStatus() override;


		protected:
			// ここにペンギン共通のステータスを追加していく
			/** 移動速度(スニーク) */
			float m_sneakSpeed;
			/** 移動速度(スライド) */
			float m_slideSpeed;
			/** ジャンプパワー */
			float m_jumpPower;
		};
	}
}

