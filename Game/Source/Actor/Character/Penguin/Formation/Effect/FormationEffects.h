/**
 * @file FormationEffects.h
 * @brief 陣形効果の具体クラス群
 * @author 竹林
 */
#pragma once
#include "IFormationEffect.h"
#include "Source/Effect/Types.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 固定速度倍率エフェクト
		 * @details GetSpeedMultiplier() に MasterFormationParameter 上の倍率をそのまま返す。
		 *          パラメーターへの生ポインタを保持するため、ホットリロードで即座に反映される。
		 *          パッシブ（0.8xなど）でもウルト（1.3xなど）でも同じクラスを使う。
		 */
		class SpeedModifierEffect : public IFormationEffect
		{
			const float* m_multiplier;

		public:
			/** @param multiplier MasterFormationParameter が保持する速度倍率フィールドへのポインタ（非所有） */
			explicit SpeedModifierEffect(const float* multiplier)
				: m_multiplier(multiplier)
			{}


			void Enter(const UltContext& ctx) override;
			void Update(float dt, const UltContext& ctx) override;
			void Exit(const UltContext& ctx) override;

			float GetSpeedMultiplier(int level) const override { return *m_multiplier; }


		private:
			EffectHandle m_ultHandle;
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
		 * @brief 渦潮近傍限定速度上昇エフェクト（防御陣形ウルト）
		 * @details Update() で渦潮との近接を判定し、近傍時のみ速度ボーナスを付与する。
		 *          ウルトチェーンに登録するため、Update/Exit はウルト中にのみ呼ばれる。
		 *          パラメーターへの生ポインタを保持するため、ホットリロードで即座に反映される。
		 */
		class WhirlpoolSpeedBoostEffect : public IFormationEffect
		{
			const float* m_multiplier;
			bool         m_isNearWhirlpool = false;

		public:
			/** @param multiplier 渦潮近傍時の速度倍率フィールドへのポインタ（非所有、例: 1.5f） */
			explicit WhirlpoolSpeedBoostEffect(const float* multiplier)
				: m_multiplier(multiplier)
			{}

			void  Update(float dt, const UltContext& ctx) override;
			void  Exit(const UltContext& ctx) override;
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
		 *          パラメーターへの生ポインタを保持するため、ホットリロードで即座に反映される。
		 */
		class PenguinCallEffect : public IFormationEffect
		{
			const float* m_callDistance;

		public:
			/** @param callDistance この距離以内の非フォロワーを呼び戻す距離フィールドへのポインタ（非所有） */
			explicit PenguinCallEffect(const float* callDistance)
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
			void Update(float dt, const UltContext& ctx) override;
			void Exit(const UltContext& ctx) override;


		private:
			/** @brief ウルト用のエフェクトハンドル */
			EffectHandle m_ultHandle;
		};
	}
}
