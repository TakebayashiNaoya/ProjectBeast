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
		class EnemyStatus : public CharacterStatus
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
			/**
			 * @brief 現在のスタミナの取得
			 * @return 現在のスタミナ
			 */
			inline float GetStamina()             const { return m_stamina; }
			/**
			 * @brief 最大スタミナの取得
			 * @return 最大スタミナ
			 */
			inline float GetMaxStamina()          const { return m_maxStamina; }
			/**
			 * @brief スタミナの消費量の取得
			 * @return スタミナの消費量
			 */
			inline float GetStaminaDrainRate()    const { return m_staminaDrainRate; }
			/**
			 * @brief ペンギンを追うのを止める距離の取得
			 * @return ペンギンを追うのを止める距離
			 */
			inline float GetLostChaseDistance()   const { return m_lostChaseDistance; }
			/**
			 * @brief スタミナが無くなったかの取得
			 * @return スタミナが無くなっていればtrue
			 */
			inline bool  IsStaminaEmpty()         const { return m_stamina <= MIN_STAMINA; }

			/**
			 * @brief スタミナを消費させる関数
			 * @param amount 消費量
			 */
			void DecreaseStamina(float amount)
			{
				m_stamina -= amount;
				if (m_stamina < 0.0f)
				{
					m_stamina = 0.0f;
				}
			}
			/**
			 * @brief スタミナ全回復
			 */
			void FullRecoverStamina()
			{
				m_stamina = m_maxStamina;
			}

			/**
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;

			void Update() override;

			EnemyStatus();
			~EnemyStatus() override;


		private:
			// --- マジックナンバー排除用の内部定数 ---
			/** 未初期化状態を表すスタミナ値 */
			static constexpr float UNINITIALIZED_STAMINA = -1.0f;
			/** スタミナの下限値 */
			static constexpr float MIN_STAMINA = 0.0f;

			// ここにエネミー固有のステータスを追加していく
			/** 歩き速度 */
			float m_walkSpeed;
			/** 最大食事数 */
			int m_maxEat;
			/** スタミナ（宣言時に未初期化フラグを入れておく） */
			float m_stamina = UNINITIALIZED_STAMINA;
			/** 最大スタミナ */
			float m_maxStamina;
			/** スタミナの消費量 */
			float m_staminaDrainRate;
			/** ペンギンを追うのを止める距離 */
			float m_lostChaseDistance;
		};
	}
}