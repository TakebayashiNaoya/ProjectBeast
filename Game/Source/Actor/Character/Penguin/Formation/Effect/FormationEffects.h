/**
 * @file FormationEffects.h
 * @brief 陣形効果の具体クラス群
 * @author 竹林
 */
#pragma once
#include "IFormationEffect.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 固定速度倍率エフェクト
		 * @details GetSpeedMultiplier() に固定の倍率を返す。
		 *          パッシブ（0.8xなど）でもウルト（1.3xなど）でも同じクラスを使う。
		 */
		class SpeedModifierEffect : public IFormationEffect
		{
			float m_multiplier;

		public:
			/** @param multiplier 速度倍率（例: 0.8f, 1.3f） */
			explicit SpeedModifierEffect(float multiplier)
				: m_multiplier(multiplier)
			{}

			float GetSpeedMultiplier(int level) const override { return m_multiplier; }
		};


		/**
		 * @brief レベル連動速度倍率エフェクト（パッシブ）
		 * @details GetSpeedMultiplier() = baseRate + coefficient * level
		 */
		class LevelSpeedEffect : public IFormationEffect
		{
			float m_baseRate;
			float m_coefficient;

		public:
			/**
			 * @param baseRate    レベル0の倍率
			 * @param coefficient レベルごとの増分
			 */
			LevelSpeedEffect(float baseRate, float coefficient)
				: m_baseRate(baseRate)
				, m_coefficient(coefficient)
			{}

			float GetSpeedMultiplier(int level) const override
			{
				return m_baseRate + m_coefficient * level;
			}
		};


		/**
		 * @brief 渦潮近傍限定速度上昇エフェクト（防御陣形ウルト）
		 * @details Update() で渦潮との近接を判定し、近傍時のみ速度ボーナスを付与する。
		 *          ウルトチェーンに登録するため、Update/Exit はウルト中にのみ呼ばれる。
		 */
		class WhirlpoolSpeedBoostEffect : public IFormationEffect
		{
			float m_multiplier;
			bool  m_isNearWhirlpool = false;

		public:
			/** @param multiplier 渦潮近傍時の速度倍率（例: 1.5f） */
			explicit WhirlpoolSpeedBoostEffect(float multiplier)
				: m_multiplier(multiplier)
			{}

			void  Update(float dt, const UltContext& ctx) override;
			void  Exit  (const UltContext& ctx) override;
			float GetSpeedMultiplier(int level) const override;
		};

		/**
		 * @brief 渦潮耐性エフェクト
		 * @details HasWhirlpoolResistance() を常に true にする。
		 *          パッシブチェーンに登録すれば常時耐性、ウルトチェーンに登録すればウルト中のみ耐性。
		 */
		class WhirlpoolResistanceEffect : public IFormationEffect
		{
		public:
			bool HasWhirlpoolResistance() const override { return true; }
		};

		/**
		 * @brief ペンギン呼び出しエフェクト
		 * @details ウルト発動時（Enter）に指定距離内の非フォロワーペンギンを陣形に呼び戻す。
		 */
		class PenguinCallEffect : public IFormationEffect
		{
			float m_callDistance;

		public:
			/** @param callDistance この距離以内の非フォロワーを呼び戻す */
			explicit PenguinCallEffect(float callDistance)
				: m_callDistance(callDistance)
			{}

			void Enter(const UltContext& ctx) override;
		};


		/**
		 * @brief シロクマ攻撃無効化エフェクト（防御陣形ウルト）
		 * @details ウルト発動中、シロクマからの攻撃ダメージを無効にする。
		 */
		class BearAttackNullifyEffect : public IFormationEffect
		{
		public:
			void Enter(const UltContext& ctx) override;
			void Exit (const UltContext& ctx) override;
		};
	}
}
