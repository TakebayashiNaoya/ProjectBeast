/**
 * @file FormationVisual.cpp
 * @brief 陣形に付属させる演出（ビジュアルエフェクト）の実装
 */
#include "stdafx.h"
#include "FormationVisual.h"
#include "FormationContext.h"

#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Effect/EffectManager.h"
#include "graphics/effect/BeastEffectEmitter.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** エフェクトのY座標オフセット */
			constexpr float OFFSET_Y = 10.0f;

			/**
			 * 陣形半径 → エフェクトスケールの換算パラメーター。
			 * BASE_RADIUSの陣形のときBASE_SCALEになり、陣形が大きくなるほど比例してスケールが上がる。
			 */
			 /** 基準となる陣形半径。この半径のときBASE_SCALEになる */
			constexpr float BASE_RADIUS = 30.0f;
			/** 基準スケール */
			constexpr float BASE_SCALE = 8.0f;
			/** スケールの下限（フォロワー0体で半径0でも演出が消えないようにする） */
			constexpr float MIN_SCALE = 4.0f;
			/** スケールの上限（陣形が巨大化しても演出が破綻しないようにする） */
			constexpr float MAX_SCALE = 100.0f;
			/** 演出が陣形半径に追従する速さ（1秒あたりの補間率） */
			constexpr float SCALE_FOLLOW_SPEED = 4.0f;


			/**
			 * @brief 陣形の最外半径からエフェクトスケールを算出する
			 * @param ctx 陣形コンテキスト（formationRadiusを使用）
			 * @return クランプ済みのスケール値
			 */
			float CalcScale(const FormationContext& ctx)
			{
				const float scale = BASE_SCALE * (ctx.formationRadius / BASE_RADIUS);
				return std::clamp(scale, MIN_SCALE, MAX_SCALE);
			}
		}


		FormationAuraVisual::FormationAuraVisual(EnEffectKind loopKind, EnEffectKind endKind)
			: m_loopKind(loopKind)
			, m_endKind(endKind)
		{
		}


		EffectHandle FormationAuraVisual::PlayAttached(EnEffectKind kind, float scale, const FormationContext& ctx)
		{
			auto& em = EffectManager::Get();
			const EffectHandle handle = em.PlayEffect(
				kind,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * scale)
			);
			em.AttachEffect(
				handle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
			return handle;
		}


		void FormationAuraVisual::Enter(const FormationContext& ctx)
		{
			m_scale = CalcScale(ctx);
			m_handle = PlayAttached(m_loopKind, m_scale, ctx);
		}


		void FormationAuraVisual::Update(float dt, const FormationContext& ctx)
		{
			// 隊列が増えて陣形が広がったら、演出も追いかけて広げる（一気に変わらないよう補間する）
			const float t = std::clamp(dt * SCALE_FOLLOW_SPEED, 0.0f, 1.0f);
			m_scale += (CalcScale(ctx) - m_scale) * t;

			auto* effect = EffectManager::Get().FindEffect(m_handle);
			if (effect == nullptr)
			{
				// 再生が終わったら出し直してループさせる
				m_handle = PlayAttached(m_loopKind, m_scale, ctx);
				return;
			}
			effect->SetScale(Vector3(Vector3::One * m_scale));
		}


		void FormationAuraVisual::Exit(const FormationContext& ctx)
		{
			EffectManager::Get().StopEffect(m_handle);
			if (m_endKind == EnEffectKind::None) return;
			// 抜けるときのctxは新しい陣形の半径になっているため、直前までの大きさで出す
			m_handle = PlayAttached(m_endKind, m_scale, ctx);
		}
	}
}
