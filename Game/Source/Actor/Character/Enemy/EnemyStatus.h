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
			/**  */
			inline float GetStamina()             const { return m_stamina; }
			inline float GetMaxStamina()          const { return m_maxStamina; }
			inline float GetStaminaDrainRate()    const { return m_staminaDrainRate; }
			inline float GetLostChaseDistance()   const { return m_lostChaseDistance; }
			inline bool  IsStaminaEmpty()         const { return m_stamina <= 0.0f; }

			// スタミナを消費させる関数
			void DecreaseStamina(float amount)
			{
				m_stamina -= amount;
				if (m_stamina < 0.0f)
				{
					m_stamina = 0.0f;
				}
			}
			/** スタミナ全回復 */
			void FullRecoverStamina()
			{
				m_stamina = m_maxStamina;
			}

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
			/** スタミナ */
			// 未初期化状態の判定用として -1.0f を入れておく
			float m_stamina = -1.0f;
			/** 最大スタミナ */
			float m_maxStamina;
			/** スタミナの消費量 */
			float m_staminaDrainRate;
			/** ペンギンを追うのを止める距離 */
			float m_lostChaseDistance;
		};
	}
}