/**
 * @file IFormationEffect.h
 * @brief 陣形効果のインターフェース
 * @author 竹林
 */
#pragma once
#include "../Ult/UltContext.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形効果のインターフェース
		 * @details
		 *   パッシブ・ウルト両方の効果クラスが実装するインターフェース。
		 *   各クラスは「自分の効果分だけ」を返す独立した実装を持つ。
		 *   FormationEffectChain が複数のエフェクトをまとめて結果を合成する。
		 */
		class IFormationEffect
		{
		public:
			virtual ~IFormationEffect() = default;

			/** @brief 移動速度倍率を返す（デフォルト: 1.0x＝効果なし） */
			virtual float GetSpeedMultiplier(int level) const { return 1.0f; }

			/** @brief 渦潮耐性を持つか（デフォルト: false） */
			virtual bool  HasWhirlpoolResistance() const { return false; }

			/** @brief ウルト発動時 */
			virtual void  Enter (const UltContext& ctx) {}

			/** @brief ウルト中の毎フレーム更新 */
			virtual void  Update(float dt, const UltContext& ctx) {}

			/** @brief ウルト終了時 */
			virtual void  Exit  (const UltContext& ctx) {}
		};
	}
}
