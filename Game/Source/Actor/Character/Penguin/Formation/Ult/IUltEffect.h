/**
 * @file IUltEffect.h
 * @brief ウルトに付属させる演出（ビジュアルエフェクト）のインターフェース
 * @author 藤谷
 */
#pragma once
#include "Source/Effect/Types.h"


namespace app
{
	namespace actor
	{
		struct UltContext;


		/**
		 * @brief ウルトに付属させる演出（ビジュアルエフェクト）のインターフェース
		 * @details
		 *   ゲームプレイ効果（FormationEffectChain）とは分離された「見た目」専用のクラス。
		 *   各 IFormation が陣形に対応する演出を1つ所有し、UltController が
		 *   発動時に Enter()、ウルト中は毎フレーム Update()、終了時に Exit() を呼ぶ。
		 *   UltContext は保持せず、毎回引数で受け取る（UltController の転送方式と統一）。
		 *
		 *   EffectManager の PlayEffect() で生成したエフェクトのハンドルを保持し、
		 *   Exit() で StopEffect() するのが基本。ただしワンショット再生のエフェクトは
		 *   Effekseer 側で寿命が管理されるため、StopEffect() 不要。
		 */
		class IUltEffect
		{
		public:
			IUltEffect() = default;
			virtual ~IUltEffect() = default;

			/**
			 * @brief ウルト発動時に呼ばれる
			 * @param ctx ウルトコンテキスト
			 */
			virtual void Enter(const UltContext& ctx) = 0;

			/**
			 * @brief ウルト中に毎フレーム呼ばれる
			 * @param dt  前フレームからの経過時間（秒）
			 * @param ctx ウルトコンテキスト
			 */
			virtual void Update(float dt, const UltContext& ctx) = 0;

			/**
			 * @brief ウルト終了時に呼ばれる
			 * @param ctx ウルトコンテキスト
			 */
			virtual void Exit(const UltContext& ctx) = 0;


		protected:
			/** ウルトエフェクトのハンドル */
			EffectHandle m_ultHandle = 0;
		};




		/****************************************/


		/**
		 * @brief 円陣ウルトの演出
		 * @details 発動中 NormalUltAuraBegin をアタッチし、終了時に NormalUltAuraEnd を単発再生する。
		 */
		class UltEffectCircle : public IUltEffect
		{
		public:
			UltEffectCircle() = default;
			~UltEffectCircle() override = default;
			void Enter(const UltContext& ctx) override final;
			void Update(float dt, const UltContext& ctx) override final;
			void Exit(const UltContext& ctx) override final;
		};




		/****************************************/


		/**
		 * @brief 密集陣（防御特化）ウルトの演出
		 * @details 発動中 BarrierBegin をアタッチし、終了時に BarrierEnd に差し替える。
		 */
		class UltEffectCluster : public IUltEffect
		{
		public:
			UltEffectCluster() = default;
			~UltEffectCluster() override = default;
			void Enter(const UltContext& ctx) override final;
			void Update(float dt, const UltContext& ctx) override final;
			void Exit(const UltContext& ctx) override final;
		};




		/****************************************/


		/**
		 * @brief 三角陣（スピード特化）ウルトの演出
		 * @details 発動中 SpeedBoostBegin をアタッチし、終了時に SpeedBoostEnd に差し替える。
		 */
		class UltEffectTriangle : public IUltEffect
		{
		public:
			UltEffectTriangle() = default;
			~UltEffectTriangle() override = default;
			void Enter(const UltContext& ctx) override final;
			void Update(float dt, const UltContext& ctx) override final;
			void Exit(const UltContext& ctx) override final;
		};




		/****************************************/


		/**
		 * @brief 散開陣（収集特化）ウルトの演出
		 * @details 発動時に CallAura を単発再生する。
		 *          CallAura は Effekseer 側で寿命が管理されるため、Exit() での StopEffect() は不要。
		 */
		class UltEffectScatter : public IUltEffect
		{
		public:
			UltEffectScatter() = default;
			~UltEffectScatter() override = default;
			void Enter(const UltContext& ctx) override final;
			void Update(float dt, const UltContext& ctx) override final;
			void Exit(const UltContext& ctx) override final;
		};
	}
}
