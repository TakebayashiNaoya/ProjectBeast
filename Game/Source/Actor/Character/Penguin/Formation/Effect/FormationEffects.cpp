/**
 * @file FormationEffects.cpp
 * @brief 陣形効果の具体クラス群の実装
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationEffects.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** エフェクトの拡大倍率 */
			constexpr float ULT_SCALE = 8.0f;
			/** エフェクトのY座標オフセット */
			constexpr float OFFSET_Y = 10.0f;
		}


		void SpeedModifierEffect::Enter(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::SpeedBoostBegin,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * ULT_SCALE)
			);


			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}


		void SpeedModifierEffect::Update(float dt, const UltContext& ctx)
		{}


		void SpeedModifierEffect::Exit(const UltContext& ctx)
		{
			auto& em = EffectManager::Get();
			em.StopEffect(m_ultHandle);
			m_ultHandle = em.PlayEffect(
				EnEffectKind::SpeedBoostEnd,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * ULT_SCALE)
			);
			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}




		/****************************************/


		void WhirlpoolSpeedBoostEffect::Update(float dt, const UltContext& ctx)
		{
			// TODO: 渦潮との近接判定を実装する
			// WhirlpoolPowerSystem や Whirlpool クラスから近傍判定APIが用意できたら置き換える
			m_isNearWhirlpool = false;
		}

		void WhirlpoolSpeedBoostEffect::Exit(const UltContext& ctx)
		{
			m_isNearWhirlpool = false;
		}

		float WhirlpoolSpeedBoostEffect::GetSpeedMultiplier(int level) const
		{
			return m_isNearWhirlpool ? *m_multiplier : 1.0f;
		}




		/****************************************/


		void PenguinCallEffect::Enter(const UltContext& ctx)
		{
			if (!ctx.penguinManager || !ctx.daddyPenguin) return;

			const Vector3 daddyPos = ctx.penguinManager->GetDaddyPosition();
			const float   distSq = (*m_callDistance) * (*m_callDistance);

			for (ChildPenguin* penguin : ctx.penguinManager->GetChildPenguin())
			{
				if (!penguin) continue;
				if (ctx.penguinManager->IsFollower(penguin)) continue;

				const Vector3 diff = penguin->GetTransform().m_position - daddyPos;
				if (diff.LengthSq() <= distSq)
				{
					ctx.penguinManager->AddFollower(penguin);
				}
			}
		}



		/****************************************/


		void BearAttackNullifyEffect::Enter(const UltContext& ctx)
		{
			// TODO: シロクマ攻撃無効化フラグを有効にする
			// DaddyPenguin や EnemyManager に無敵フラグのAPIが用意できたら実装する
			auto& em = EffectManager::Get();
			m_ultHandle = em.PlayEffect(
				EnEffectKind::BarrierBegin,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * ULT_SCALE)
			);

			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}

		void BearAttackNullifyEffect::Update(float dt, const UltContext& ctx)
		{}

		void BearAttackNullifyEffect::Exit(const UltContext& ctx)
		{
			// TODO: シロクマ攻撃無効化フラグを解除する


			// BarrierLoopエフェクトを終了し、BarrierEndエフェクトを再生
			auto& em = EffectManager::Get();
			em.StopEffect(m_ultHandle);
			m_ultHandle = em.PlayEffect(
				EnEffectKind::BarrierEnd,
				ctx.daddyPenguin->GetTransform().m_position,
				ctx.daddyPenguin->GetTransform().m_rotation,
				Vector3(Vector3::One * ULT_SCALE)
			);

			em.AttachEffect(
				m_ultHandle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
		}
	}
}
