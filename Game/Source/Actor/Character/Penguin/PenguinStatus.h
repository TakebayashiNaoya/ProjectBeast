/**
 * @file PenguinStatus.h
 * @brief ペンギンのステータス
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
			 * @brief 最大体力を取得
			 * @return 最大体力
			 */
			inline int GetMaxHp() const { return m_maxHp; }
			/**
			 * @brief 体力を取得
			 * @return 体力
			 */
			inline int GetHp() const { return m_hp; }
			/**
			 * @brief ダメージ処理
			 */
			inline void Damage(const int dmg = 1)
			{
				m_hp = std::max<int>(0, m_hp - dmg);
			}
			/**
			 * @brief 死んでいるか
			 * @return 死んでいるか
			 */
			inline bool IsDead() const { return m_hp <= 0; }
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
			/**
			 * @brief ジャンプのスタミナ最大値を取得
			 * @return ジャンプのスタミナ最大値
			 */
			inline float GetJumpStaminaMax() const { return m_jumpStaminaMax; }
			/**
			 * @brief ジャンプのスタミナ回復速度を取得
			 * @return ジャンプのスタミナ回復速度(1秒あたり)
			 */
			inline float GetJumpStaminaRecoverSpeed() const { return m_jumpStaminaRecoverSpeed; }
			/**
			 * @brief スライドのスタミナ最大値を取得
			 * @return スライドのスタミナ最大値
			 */
			inline float GetSlideStaminaMax() const { return m_slideStaminaMax; }
			/**
			 * @brief スライドのスタミナ減少速度を取得
			 * @return スライドのスタミナ減少速度(1秒あたり)
			 */
			inline float GetSlideStaminaDecreaseSpeed() const { return m_slideStaminaDecreaseSpeed; }
			/**
			 * @brief スライドのスタミナ回復速度を取得
			 * @return スライドのスタミナ回復速度(1秒あたり)
			 */
			inline float GetSlideStaminaRecoverSpeed() const { return m_slideStaminaRecoverSpeed; }
			/**
			 * @brief ノイズレベルを取得 (0.0f ～ 1.0f)
			 * @return ノイズレベル
			 */
			inline float GetNoiseLevel() const { return m_noiseLevel; }
			/**
			 * @brief ノイズレベルを設定
			 * @param noise ノイズレベル (0.0f ～ 1.0f)
			 */
			inline void SetNoiseLevel(const float noise) { m_noiseLevel = noise; }


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
			/** 最大体力 */
			int m_maxHp;
			/** 体力 */
			int m_hp;
			/** 移動速度(スニーク) */
			float m_sneakSpeed;
			/** 移動速度(スライド) */
			float m_slideSpeed;
			/** ジャンプパワー */
			float m_jumpPower;
			/** ジャンプのスタミナ最大値 */
			float m_jumpStaminaMax;
			/** ジャンプのスタミナ回復速度(1秒あたり) */
			float m_jumpStaminaRecoverSpeed;
			/** スライドのスタミナ最大値 */
			float m_slideStaminaMax;
			/** スライドのスタミナ減少速度(1秒あたり) */
			float m_slideStaminaDecreaseSpeed;
			/** スライドのスタミナ回復速度(1秒あたり) */
			float m_slideStaminaRecoverSpeed;
			/** ノイズレベル(音の大きさや見つかりやすさの指標) */
			float m_noiseLevel = 0.0f;
		};
	}
}