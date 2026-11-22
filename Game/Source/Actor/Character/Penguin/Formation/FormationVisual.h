/**
 * @file FormationVisual.h
 * @brief 陣形に付属させる演出（ビジュアルエフェクト）のインターフェース
 */
#pragma once
#include "Source/Effect/Types.h"


namespace app
{
	namespace actor
	{
		struct FormationContext;


		/**
		 * @brief 陣形に付属させる演出（ビジュアルエフェクト）のインターフェース
		 * @details
		 *   ゲームプレイ効果（FormationEffectChain）とは分離された「見た目」専用のクラス。
		 *   FormationControllerが陣形に入ったときにEnter()、陣形中は毎フレームUpdate()、
		 *   陣形を抜けたときにExit()を呼ぶ。FormationContextは保持せず、毎回引数で受け取る。
		 *
		 *   EffectManagerのPlayEffect()で生成したエフェクトのハンドルを保持し、
		 *   Exit()でStopEffect()するのが基本。
		 */
		class IFormationVisual
		{
		public:
			IFormationVisual() = default;
			virtual ~IFormationVisual() = default;

			/**
			 * @brief 陣形に入ったときに呼ばれる
			 * @param ctx 陣形コンテキスト
			 */
			virtual void Enter(const FormationContext& ctx) = 0;

			/**
			 * @brief 陣形中に毎フレーム呼ばれる
			 * @param dt  前フレームからの経過時間（秒）
			 * @param ctx 陣形コンテキスト
			 */
			virtual void Update(float dt, const FormationContext& ctx) = 0;

			/**
			 * @brief 陣形を抜けたときに呼ばれる
			 * @param ctx 陣形コンテキスト
			 */
			virtual void Exit(const FormationContext& ctx) = 0;


		protected:
			/** 再生中のエフェクトのハンドル */
			EffectHandle m_handle = 0;
		};




		/****************************************/


		/**
		 * @brief 陣形中ずっと親に付けて出す演出
		 * @details 再生が終わったら出し直してループさせる。抜けたときにendKindがあれば差し替える。
		 *          Update()で陣形の最外半径に合わせてスケールを補間する（隊列が増えると広がる）。
		 */
		class FormationAuraVisual : public IFormationVisual
		{
		public:
			/**
			 * @param loopKind 陣形中ずっと出すエフェクト
			 * @param endKind  陣形を抜けたときに出すエフェクト（出さないならNone）
			 */
			FormationAuraVisual(EnEffectKind loopKind, EnEffectKind endKind);
			~FormationAuraVisual() override = default;
			void Enter(const FormationContext& ctx) override final;
			void Update(float dt, const FormationContext& ctx) override final;
			void Exit(const FormationContext& ctx) override final;

		private:
			/** @brief kindを親に付けて再生する */
			EffectHandle PlayAttached(EnEffectKind kind, float scale, const FormationContext& ctx);

		private:
			EnEffectKind m_loopKind;
			EnEffectKind m_endKind;
			/** 現在のエフェクトスケール（陣形半径から求めた目標値へ補間する） */
			float m_scale = 0.0f;
		};
	}
}
