/**
 * @file EnemyStatus.h
 * @brief エネミーのステータス
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief エネミーのステータスクラス
		 */
		class EnemyStatus :public CharacterStatus
		{
		public:
			// ここにエネミー固有のステータス用のゲッター関数を追加していく
			/**
			 * @brief 歩き速度を取得
			 * @return 歩き速度
			 */
			inline float GetWalkSpeed() const { return m_walkSpeed; }
			/**
			 * @brief 最大食事数を取得
			 * @return 最大食事数
			 */
			inline int GetMaxEat()const { return m_maxEat; }

		public:
			/**
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


		public:
			EnemyStatus();
			~EnemyStatus() override;


		private:
			// ここにエネミー固有のステータスを追加していく
			/** 歩き速度 */
			float m_walkSpeed;
			/** 最大食事数 */
			int m_maxEat;
		};
	}
}