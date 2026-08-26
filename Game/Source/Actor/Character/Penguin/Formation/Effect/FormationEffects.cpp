/**
 * @file FormationEffects.cpp
 * @brief 陣形効果の具体クラス群の実装
 */
#include "stdafx.h"
#include "FormationEffects.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Nature/WhirlpoolParameter.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 円陣ウルトの呼び戻し時に、呼ばれた子の足元へ出すバーストのスケール */
			const Vector3 CALL_BURST_EFFECT_SCALE(30.0f, 30.0f, 30.0f);
		}


		void WhirlpoolSpeedBoostEffect::Update(float dt, const UltContext& ctx)
		{
			m_isNearWhirlpool = false;
			if (!ctx.penguinManager) return;

			const auto* wpParam = core::ParameterManager::Get()->GetParameter<nature::MasterWhirlpoolParameter>();
			const float baseRadius = (wpParam != nullptr) ? wpParam->whirlpoolRadius : 0.0f;
			if (baseRadius <= 0.0f) return;

			const Vector3 daddyPos = ctx.penguinManager->GetDaddyPosition();

			// 親ペンギンがいずれかの渦潮の実効半径（Bigger中はスケール比率で拡大）の中にいるか判定する
			nature::WhirlpoolManager::GetInstance()->ForEach([&](nature::Whirlpool* wp)
				{
					if (m_isNearWhirlpool) return;
					if (wp->GetState() == nature::Whirlpool::EnWhirlpoolState::None) return;

					float effectiveRadius = baseRadius;
					if (wp->GetState() == nature::Whirlpool::EnWhirlpoolState::Bigger)
					{
						const float currentScaleXZ = wp->GetTransform().m_scale.x;
						const float maxScaleXZ     = wp->GetMaxScaleXZ();
						const float ratio          = (maxScaleXZ > 0.0f) ? (currentScaleXZ / maxScaleXZ) : 1.0f;
						effectiveRadius = baseRadius * ratio;
					}

					Vector3 diff = daddyPos - wp->GetTransform().m_position;
					diff.y = 0.0f;
					if (diff.LengthSq() <= effectiveRadius * effectiveRadius)
					{
						m_isNearWhirlpool = true;
					}
				});
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
			const float   distSq   = (*m_callDistance) * (*m_callDistance);

			for (ChildPenguin* penguin : ctx.penguinManager->GetChildPenguin())
			{
				if (!penguin) continue;
				if (ctx.penguinManager->IsFollower(penguin)) continue;

				const Vector3 diff = penguin->GetTransform().m_position - daddyPos;
				if (diff.LengthSq() <= distSq)
				{
					ctx.penguinManager->AddFollower(penguin);

					/** 呼び戻された子の足元にバーストを出して「集まってくる」絵を作る。
					 *  入隊した子はウルト中の隊列発光でも光るため、二段の演出になる */
					EffectManager::Get().PlayEffect(
						EnEffectKind::DaddyPenguinCommand,
						penguin->GetTransform().m_position,
						Quaternion::Identity,
						CALL_BURST_EFFECT_SCALE);
				}
			}
		}




		/****************************************/


		void BearAttackNullifyEffect::Enter(const UltContext& ctx)
		{
			/** 無効化の判定と反撃（ノックバック＋スタン）は EnemyController の
			 *  攻撃ヒット処理側で行う。ここではフラグを立てるだけ */
			BattleManager::GetInstance().SetBearAttackNullified(true);
		}


		void BearAttackNullifyEffect::Exit(const UltContext& ctx)
		{
			BattleManager::GetInstance().SetBearAttackNullified(false);
		}
	}
}
