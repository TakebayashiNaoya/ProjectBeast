/**
 * @file UltEffectDecorators.h
 * @brief ウルト効果の具体デコレーター群
 * @author 竹林
 */
#pragma once
#include "IUltEffect.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 速度上昇デコレーター
		 * @details ウルト中、移動速度を指定した割合だけ上昇させる。
		 *          GetSpeedMultiplierBonus() が wrapped の値に乗算して返す。
		 */
		class SpeedBoostDecorator : public UltEffectDecorator
		{
			float m_boostRate;  /** 倍率（例: boostPercent=30 → 1.3f） */

		public:
			/**
			 * @param w            ラップ対象
			 * @param boostPercent 上昇率（%）。30 なら 30%UP
			 */
			SpeedBoostDecorator(std::unique_ptr<IUltEffect> w, float boostPercent)
				: UltEffectDecorator(std::move(w))
				, m_boostRate(1.0f + boostPercent / 100.0f)
			{}

			float GetSpeedMultiplierBonus() const override
			{
				return m_wrapped->GetSpeedMultiplierBonus() * m_boostRate;
			}
		};


		/**
		 * @brief 渦潮免疫デコレーター（ウルト中）
		 * @details ウルト中に全フォロワーを渦潮捕獲から守る。
		 */
		class WhirlpoolImmunityDecorator : public UltEffectDecorator
		{
			bool m_enabled;

		public:
			/**
			 * @param w       ラップ対象
			 * @param enabled false にすれば無効化したまま組み込める
			 */
			WhirlpoolImmunityDecorator(std::unique_ptr<IUltEffect> w, bool enabled = true)
				: UltEffectDecorator(std::move(w))
				, m_enabled(enabled)
			{}

			bool IsWhirlpoolImmune() const override
			{
				return m_enabled || m_wrapped->IsWhirlpoolImmune();
			}
		};


		/**
		 * @brief ペンギン呼び出しデコレーター
		 * @details ウルト発動時、指定距離内の非フォロワーペンギンを陣形に呼び戻す。
		 */
		class PenguinCallDecorator : public UltEffectDecorator
		{
			float m_callDistance;  /** 呼び戻し距離 */

		public:
			/**
			 * @param w            ラップ対象
			 * @param callDistance この距離以内の非フォロワーを呼び戻す
			 */
			PenguinCallDecorator(std::unique_ptr<IUltEffect> w, float callDistance)
				: UltEffectDecorator(std::move(w))
				, m_callDistance(callDistance)
			{}

			void Activate(const UltContext& ctx) override;
		};


		/**
		 * @brief 渦潮内限定速度上昇デコレーター（防御陣形専用）
		 * @details ウルト中、近隣に渦潮が存在する間だけ速度ボーナスを付与する。
		 *          Update() で渦潮との近接を毎フレーム判定し、ボーナスを切り替える。
		 */
		class WhirlpoolSpeedBoostDecorator : public UltEffectDecorator
		{
			float m_boostRate;
			bool  m_isNearWhirlpool = false;  /** 現フレームで渦潮近傍にいるか */

		public:
			/**
			 * @param w            ラップ対象
			 * @param boostPercent 渦潮近傍時の速度上昇率（%）
			 */
			WhirlpoolSpeedBoostDecorator(std::unique_ptr<IUltEffect> w, float boostPercent)
				: UltEffectDecorator(std::move(w))
				, m_boostRate(1.0f + boostPercent / 100.0f)
			{}

			void  Update(float dt, const UltContext& ctx) override;
			void  Deactivate(const UltContext& ctx) override;
			float GetSpeedMultiplierBonus() const override;
		};


		/**
		 * @brief シロクマ攻撃無効化デコレーター
		 * @details ウルト中、シロクマからの攻撃ダメージを無効にする。
		 */
		class BearAttackNullifyDecorator : public UltEffectDecorator
		{
		public:
			explicit BearAttackNullifyDecorator(std::unique_ptr<IUltEffect> w)
				: UltEffectDecorator(std::move(w))
			{}

			void Activate(const UltContext& ctx) override;
			void Deactivate(const UltContext& ctx) override;
		};
	}
}
