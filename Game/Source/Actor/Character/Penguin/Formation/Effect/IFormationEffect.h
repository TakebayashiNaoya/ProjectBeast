/**
 * @file IFormationEffect.h
 * @brief 陣形効果のインターフェース
 * @author 竹林
 */
#pragma once
#include "Source/Actor/Character/Penguin/Formation/Ult/UltContext.h"


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

			/** 
			 * @brief 移動速度倍率を返す
			 * @param level レベルに応じたスピード倍率
			 */
			virtual float GetSpeedMultiplier(int level) const { return 1.0f; }

			/** 
			 * @brief 渦潮耐性を持つか
			 * @return true で耐性あり、false で耐性なし
			 */
			virtual bool  HasWhirlpoolResistance() const { return false; }

			/** 
			 * @brief ウルト発動時
			 * @param ctx ウルトコンテキスト
			 */
			virtual void  Enter (const UltContext& ctx) {}

			/** 
			 * @brief ウルト中の毎フレーム更新
			 * @param dt  デルタタイム（秒）
			 * @param ctx ウルトコンテキスト
			 */
			virtual void  Update(float dt, const UltContext& ctx) {}

			/** 
			 * @brief ウルト終了時
			 * @param ctx ウルトコンテキスト
			 */
			virtual void  Exit  (const UltContext& ctx) {}
		};
	}
}
