/**
 * @file IUltEffect.cpp
 * @brief ウルトに付属させる演出（ビジュアルエフェクト）のインターフェース
 */
#include "stdafx.h"
#include "IUltEffect.h"
#include "UltContext.h"

#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Effect/EffectManager.h"


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
			 * BASE_RADIUS の陣形のとき BASE_SCALE（従来の固定値 8.0f と同じ見た目）になり、
			 * 陣形が大きくなるほど比例してスケールが上がる。
			 */
			 /** 基準となる陣形半径。この半径のとき BASE_SCALE になる */
			constexpr float BASE_RADIUS = 30.0f;
			/** 基準スケール（従来の固定スケールと同じ値） */
			constexpr float BASE_SCALE = 8.0f;
			/** スケールの下限（フォロワー0体で半径0でも演出が消えないようにする） */
			constexpr float MIN_SCALE = 4.0f;
			/** スケールの上限（陣形が巨大化しても演出が破綻しないようにする） */
			constexpr float MAX_SCALE = 100.0f;


			/**
			 * @brief 陣形の最外半径からエフェクトスケールを算出する
			 * @param ctx ウルトコンテキスト（formationRadius を使用）
			 * @return クランプ済みのスケール値
			 */
			float CalcScale(const UltContext& ctx)
			{
				const float scale = BASE_SCALE * (ctx.formationRadius / BASE_RADIUS);
				return std::clamp(scale, MIN_SCALE, MAX_SCALE);
			}
		}




		/****************************************/


		void UltEffectCircle::Enter(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::CircleUltBegin,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}


		void UltEffectCircle::Update(float dt, const UltContext& ctx)
		{}


		void UltEffectCircle::Exit(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			em.StopEffect(m_ultHandle);
			m_ultHandle = em.PlayEffect(
				EnEffectKind::CircleUltEnd,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}




		/****************************************/


		void UltEffectCluster::Enter(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::ClusterUltBegin,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}


		void UltEffectCluster::Update(float dt, const UltContext& ctx)
		{}


		void UltEffectCluster::Exit(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			em.StopEffect(m_ultHandle);
			m_ultHandle = em.PlayEffect(
				EnEffectKind::ClusterUltEnd,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}




		/****************************************/


		void UltEffectTriangle::Enter(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::TriangleUltBegin,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);

			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}


		void UltEffectTriangle::Update(float dt, const UltContext& ctx)
		{}


		void UltEffectTriangle::Exit(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			em.StopEffect(m_ultHandle);
			m_ultHandle = em.PlayEffect(
				EnEffectKind::TriangleUltEnd,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}




		/****************************************/


		void UltEffectScatter::Enter(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::ScatterUlt,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * CalcScale(ctx))
			);

			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}


		void UltEffectScatter::Update(float dt, const UltContext& ctx)
		{}


		void UltEffectScatter::Exit(const UltContext& ctx)
		{
			// CallAura は Effekseer 側で寿命が管理されるため、StopEffect() は不要
		}
	}
}
