/**
 * @file CharacterStatus.h
 * @brief キャラクターのステータス基底クラス
 */
#pragma once
#include "Source/Actor/ActorStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief キャラクターのステータス基底クラス
		 */
		class CharacterStatus : public ActorStatus
		{
		public:
			/**
			 * @brief 走り速度を取得
			 * @return 走り速度
			 */
			inline float GetRunSpeed() const { return m_runSpeed; }
			/**
			 * @brief 泳ぎ速度を取得
			 * @return 泳ぎ速度
			 */
			inline float GetSwimSpeed() const { return m_swimSpeed; }
			/**
			 * @brief 半径を取得
			 * @return 半径
			 */
			inline float GetRadius() const { return m_radius; }
			/**
			 * @brief 高さを取得
			 * @return 高さ
			 */
			inline float GetHeight() const { return m_height; }


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			virtual void Setup() override;


			virtual void Update() override;


		public:
			CharacterStatus();
			virtual ~CharacterStatus() override = default;


		protected:
			/** 移動速度(走り) */
			float m_runSpeed;
			/** 移動速度(泳ぎ) */
			float m_swimSpeed;
			/** 半径 */
			float m_radius;
			/** 高さ */
			float m_height;
		};
	}
}