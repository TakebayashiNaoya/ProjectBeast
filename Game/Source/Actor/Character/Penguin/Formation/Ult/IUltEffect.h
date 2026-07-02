/**
 * @file IUltEffect.h
 * @brief ウルト効果のインターフェース・基底・抽象デコレーター
 * @author 竹林
 */
#pragma once
#include <memory>
#include "UltContext.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ウルト効果のインターフェース
		 * @details
		 *   Activate/Update/Deactivate でライフサイクルを管理する。
		 *   速度ボーナスや渦潮免疫はゲッターで毎フレーム参照される。
		 */
		class IUltEffect
		{
		public:
			virtual ~IUltEffect() = default;

			/**
			 * @brief ウルト発動時
			 * @param ctx ウルト発動時のコンテキスト情報
			 */
			virtual void Activate(const UltContext& ctx) {}

			/**
			 * @brief ウルト中の毎フレーム更新
			 * @param dt  前フレームからの経過時間（秒）
			 * @param ctx ウルト更新時のコンテキスト情報
			 */
			virtual void Update(float dt, const UltContext& ctx) {}

			/**
			 * @brief ウルト終了時
			 * @param ctx ウルト終了時のコンテキスト情報
			 */
			virtual void Deactivate(const UltContext& ctx) {}

			/**
			 * @brief ウルト中の速度倍率ボーナス（乗算）。デフォルト 1.0x（効果なし）
			 * @return 速度倍率ボーナス（例: 1.3f なら 30%UP）
			 */
			virtual float GetSpeedMultiplierBonus() const { return 1.0f; }

			/**
			 * @brief ウルト中に渦潮免疫を付与するか
			 * @return true で有効、false で無効
			 */
			virtual bool IsWhirlpoolImmune() const { return false; }
		};


		/**
		 * @brief 効果なし（デコレーターチェーンの末端）
		 * @details 何もしないデフォルト実装。
		 */
		class BaseUlt : public IUltEffect {};


		/**
		 * @brief ウルトデコレーターの抽象基底
		 * @details オーバーライドしないメソッドは自動的に m_wrapped に委譲される。
		 */
		class UltEffectDecorator : public IUltEffect
		{
		public:
			explicit UltEffectDecorator(std::unique_ptr<IUltEffect> w)
				: m_wrapped(std::move(w))
			{}

			void  Activate(const UltContext& ctx) override         { m_wrapped->Activate(ctx); }
			void  Update(float dt, const UltContext& ctx) override { m_wrapped->Update(dt, ctx); }
			void  Deactivate(const UltContext& ctx) override       { m_wrapped->Deactivate(ctx); }
			float GetSpeedMultiplierBonus() const override         { return m_wrapped->GetSpeedMultiplierBonus(); }
			bool  IsWhirlpoolImmune()       const override         { return m_wrapped->IsWhirlpoolImmune(); }


		protected:
			std::unique_ptr<IUltEffect> m_wrapped;
		};
	}
}
