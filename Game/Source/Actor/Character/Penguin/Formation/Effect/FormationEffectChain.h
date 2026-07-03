/**
 * @file FormationEffectChain.h
 * @brief 複数の陣形効果をまとめて管理するチェーン
 * @author 竹林
 */
#pragma once
#include <memory>
#include <vector>
#include "IFormationEffect.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形効果チェーン
		 * @details
		 *   AddEffect() で任意のエフェクトを追加し、
		 *   GetSpeedMultiplier() は全エフェクトの乗算、
		 *   HasWhirlpoolResistance() は全エフェクトの OR で結果を合成する。
		 *   Enter/Update/Exit は登録された全エフェクトに順番に転送する。
		 *
		 *   パッシブチェーン（常時）とウルトチェーン（発動時のみ）の
		 *   2本を FormationController が使い分ける。
		 */
		class FormationEffectChain
		{
		public:
			/**
			 * @brief エフェクトを追加する
			 * @param effect 追加する陣形効果（所有権をチェーンに移す）
			 */
			void AddEffect(std::unique_ptr<IFormationEffect> effect)
			{
				m_effects.push_back(std::move(effect));
			}

			/**
			 * @brief 全エフェクトの速度倍率を乗算して返す
			 * @param level 現在の陣形レベル
			 * @return 合成後の速度倍率（エフェクトなしは 1.0）
			 */
			float GetSpeedMultiplier(int level) const
			{
				float result = 1.0f;
				for (const auto& e : m_effects)
				{
					result *= e->GetSpeedMultiplier(level);
				}
				return result;
			}

			/**
			 * @brief いずれかのエフェクトが渦潮耐性を持つか
			 * @return true で耐性あり
			 */
			bool HasWhirlpoolResistance() const
			{
				for (const auto& e : m_effects)
				{
					if (e->HasWhirlpoolResistance()) return true;
				}
				return false;
			}

			/** @brief 全エフェクトの Enter を呼ぶ（ウルト発動時） */
			void Enter(const UltContext& ctx)
			{
				for (auto& e : m_effects) e->Enter(ctx);
			}

			/** @brief 全エフェクトの Update を呼ぶ（ウルト中・毎フレーム） */
			void Update(float dt, const UltContext& ctx)
			{
				for (auto& e : m_effects) e->Update(dt, ctx);
			}

			/** @brief 全エフェクトの Exit を呼ぶ（ウルト終了時） */
			void Exit(const UltContext& ctx)
			{
				for (auto& e : m_effects) e->Exit(ctx);
			}


		private:
			std::vector<std::unique_ptr<IFormationEffect>> m_effects;
		};
	}
}
