/**
 * @file FormationEffects.h
 * @brief 陣形効果の具体クラス群
 */
#pragma once
#include "IFormationEffect.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 固定速度倍率エフェクト
		 * @details GetSpeedMultiplier() に MasterFormationParameter 上の倍率をそのまま返す。
		 *          パラメーターへの生ポインタを保持するため、ホットリロードで即座に反映される。
				 */
		class SpeedModifierEffect : public IFormationEffect
		{
			const float* m_multiplier;

		public:
			/** @param multiplier MasterFormationParameter が保持する速度倍率フィールドへのポインタ（非所有） */
			explicit SpeedModifierEffect(const float* multiplier)
				: m_multiplier(multiplier)
			{}

			float GetSpeedMultiplier(int level) const override { return *m_multiplier; }
		};


		/**
		 * @brief レベル連動速度倍率エフェクト（パッシブ）
		 * @details GetSpeedMultiplier() = *baseRate + *coefficient * level
		 *          パラメーターへの生ポインタを保持するため、ホットリロードで即座に反映される。
		 */
		class LevelSpeedEffect : public IFormationEffect
		{
			const float* m_baseRate;
			const float* m_coefficient;

		public:
			/**
			 * @param baseRate    レベル0の倍率フィールドへのポインタ（非所有）
			 * @param coefficient レベルごとの増分フィールドへのポインタ（非所有）
			 */
			LevelSpeedEffect(const float* baseRate, const float* coefficient)
				: m_baseRate(baseRate)
				, m_coefficient(coefficient)
			{}

			float GetSpeedMultiplier(int level) const override
			{
				return *m_baseRate + *m_coefficient * level;
			}
		};


		/**
		 * @brief 渦潮耐性エフェクト
		 * @details HasWhirlpoolResistance() を常に true にする。
		 *          パッシブチェーンに登録すると、その陣形の間ずっと耐性を持つ。
		 */
		class WhirlpoolResistanceEffect : public IFormationEffect
		{
		public:
			bool HasWhirlpoolResistance() const override { return true; }
		};


		/**
		 * @brief シロクマ・シャチの攻撃無効化エフェクト（密集陣パッシブ）
		 * @details 密集陣でいる間、隊列の子へのシロクマ・シャチの攻撃を無効にする。
		 *          FormationControllerが陣形の切り替え時にEnter/Exitを呼ぶ。
		 */
		class BearAttackNullifyEffect : public IFormationEffect
		{
		public:
			void Enter(const FormationContext& ctx) override;
			void Exit(const FormationContext& ctx) override;
		};
	}
}
