/**
 * @file UltEffectDecorators.cpp
 * @brief ウルト効果の具体デコレーター群
 * @author 竹林
 */
#include "stdafx.h"
#include "UltEffectDecorators.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"


namespace app
{
	namespace actor
	{
		void PenguinCallDecorator::Activate(const UltContext& ctx)
		{
			UltEffectDecorator::Activate(ctx);

			if (!ctx.penguinManager || !ctx.daddyPenguin) return;

			const Vector3 daddyPos = ctx.penguinManager->GetDaddyPosition();
			const float   distSq   = m_callDistance * m_callDistance;

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




		/********************************************/


		void WhirlpoolSpeedBoostDecorator::Update(float dt, const UltContext& ctx)
		{
			UltEffectDecorator::Update(dt, ctx);

			// TODO: 渦潮との近接判定を実装する
			// WhirlpoolPowerSystem や Whirlpool クラスから近傍判定APIが用意できたら置き換える
			// 例: m_isNearWhirlpool = WhirlpoolPowerSystem::IsNearWhirlpool(ctx.daddyPenguin);
			m_isNearWhirlpool = false;
		}


		void WhirlpoolSpeedBoostDecorator::Deactivate(const UltContext& ctx)
		{
			m_isNearWhirlpool = false;
			UltEffectDecorator::Deactivate(ctx);
		}


		float WhirlpoolSpeedBoostDecorator::GetSpeedMultiplierBonus() const
		{
			const float base = m_wrapped->GetSpeedMultiplierBonus();
			return m_isNearWhirlpool ? base * m_boostRate : base;
		}




		/********************************************/


		void BearAttackNullifyDecorator::Activate(const UltContext& ctx)
		{
			UltEffectDecorator::Activate(ctx);

			// TODO: シロクマ攻撃無効化フラグを有効にする
			// DaddyPenguin や EnemyManager に無敵フラグのAPIが用意できたら実装する
		}


		void BearAttackNullifyDecorator::Deactivate(const UltContext& ctx)
		{
			// TODO: シロクマ攻撃無効化フラグを解除する

			UltEffectDecorator::Deactivate(ctx);
		}
	}
}
