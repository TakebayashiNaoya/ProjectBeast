/**
 * @file FormationPassiveDecorators.h
 * @brief 陣形パッシブ効果の具体デコレーター群
 * @author 竹林
 */
#pragma once
#include "IFormationPassive.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 渦潮耐性パッシブデコレーター
		 * @details 有効化すると HasWhirlpoolResistance() が true を返す。
		 *          直近の DefenseFormation のハードコードをデコレーターに移行したもの。
		 */
		class WhirlpoolResistancePassiveDecorator : public FormationPassiveDecorator
		{
		public:
			explicit WhirlpoolResistancePassiveDecorator(std::unique_ptr<IFormationPassive> w)
				: FormationPassiveDecorator(std::move(w))
			{}

			bool HasWhirlpoolResistance() const override { return true; }
		};


		/**
		 * @brief 速度倍率パッシブデコレーター
		 * @details GetSpeedMultiplier(level) = wrapped の結果 × (baseRate + levelCoefficient × level)
		 *
		 *  使用例:
		 *   - 固定0.8倍    : SpeedModifierPassiveDecorator(w, 0.8f)
		 *   - レベル依存   : SpeedModifierPassiveDecorator(w, 1.0f, 0.1f) → 1.0 + level*0.1
		 */
		class SpeedModifierPassiveDecorator : public FormationPassiveDecorator
		{
			float m_baseRate;           /** 速度倍率の基準値 */
			float m_levelCoefficient;   /** レベルごとの加算係数 */

		public:
			SpeedModifierPassiveDecorator(
				std::unique_ptr<IFormationPassive> w,
				float baseRate,
				float levelCoefficient = 0.0f
			)
				: FormationPassiveDecorator(std::move(w))
				, m_baseRate(baseRate)
				, m_levelCoefficient(levelCoefficient)
			{}

			float GetSpeedMultiplier(int level) const override
			{
				return m_wrapped->GetSpeedMultiplier(level) * (m_baseRate + m_levelCoefficient * level);
			}
		};
	}
}
