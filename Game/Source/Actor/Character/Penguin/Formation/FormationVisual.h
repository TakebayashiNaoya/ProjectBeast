/**
 * @file FormationVisual.h
 * @brief 陣形に付属させる演出（ビジュアルエフェクト）のインターフェース
 */
#pragma once
#include "Source/Effect/Types.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		struct FormationContext;
		struct MasterFormationParameter;


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


		/** @brief 演出の大きさを何の円に合わせるか */
		enum class EnAuraFit : uint8_t
		{
			OuterRadius,	// 隊列のいちばん外の半径
			FetchRange,		// 迎えの範囲（基本+増分×陣形レベル）
		};


		/**
		 * @brief 陣形中ずっと親に付けて出す演出
		 * @details 再生が終わったら出し直してループさせる。抜けたときにendKindがあれば差し替える。
		 *          大きさは「合わせる円の半径÷efkの大きさ1のときの半径」で出し、毎フレーム補間する。
		 *          efkの半径はFormationVisualTuning.jsonで陣形ごとに調整する。
		 */
		class FormationAuraVisual : public IFormationVisual
		{
		public:
			/**
			 * @param type     どの陣形の演出か（調整値と合わせ円の色を選ぶ）
			 * @param loopKind 陣形中ずっと出すエフェクト
			 * @param endKind  陣形を抜けたときに出すエフェクト（出さないならNone）
			 * @param fit      大きさを合わせる円
			 * @param param    陣形のパラメーター（FetchRangeのときに迎えの範囲を読む）
			 */
			FormationAuraVisual(EnFormationType type, EnEffectKind loopKind, EnEffectKind endKind, EnAuraFit fit, const MasterFormationParameter& param);
			~FormationAuraVisual() override = default;
			void Enter(const FormationContext& ctx) override final;
			void Update(float dt, const FormationContext& ctx) override final;
			void Exit(const FormationContext& ctx) override final;

		private:
			/** @brief kindを親に付けて再生する */
			EffectHandle PlayAttached(EnEffectKind kind, float scale, const FormationContext& ctx);
			/** @brief 大きさを合わせる円の半径 */
			float CalcFitRadius(const FormationContext& ctx) const;
			/** @brief 合わせる円の半径からエフェクトの大きさを出す */
			float CalcScale(const FormationContext& ctx) const;

		private:
			EnFormationType m_type;
			EnEffectKind m_loopKind;
			EnEffectKind m_endKind;
			EnAuraFit m_fit;
			const MasterFormationParameter& m_param;
			/** 現在のエフェクトスケール（陣形半径から求めた目標値へ補間する） */
			float m_scale = 0.0f;
		};
	}
}
