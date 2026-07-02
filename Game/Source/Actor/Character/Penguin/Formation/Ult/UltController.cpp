/**
 * @file UltController.cpp
 * @brief ウルトの発動・タイマー・クールダウンを管理するコントローラー
 * @author 竹林
 */
#include "stdafx.h"
#include "UltController.h"
#include "IUltEffect.h"


namespace app
{
	namespace actor
	{
		void UltController::SetUlt(IUltEffect* ult, float duration, float cooldown)
		{
			m_ult      = ult;
			m_duration = duration;
			m_cooldown = cooldown;
		}


		bool UltController::CanActivate() const
		{
			return m_ult && !m_isActive && m_cooldownTimer <= 0.0f;
		}


		void UltController::Activate(const UltContext& ctx)
		{
			if (!CanActivate()) return;

			m_isActive = true;
			m_timer    = m_duration;
			m_ult->Activate(ctx);
		}


		void UltController::Update(float dt, const UltContext& ctx)
		{
			if (m_cooldownTimer > 0.0f)
			{
				m_cooldownTimer -= dt;
			}

			if (!m_isActive) return;

			m_timer -= dt;
			m_ult->Update(dt, ctx);

			if (m_timer <= 0.0f)
			{
				m_isActive      = false;
				m_cooldownTimer = m_cooldown;
				m_ult->Deactivate(ctx);
			}
		}


		float UltController::GetCooldownRate() const
		{
			if (m_cooldown <= 0.0f) return 0.0f;
			return max(m_cooldownTimer / m_cooldown, 0.0f);
		}


		float UltController::GetSpeedMultiplierBonus() const
		{
			return (m_isActive && m_ult) ? m_ult->GetSpeedMultiplierBonus() : 1.0f;
		}


		bool UltController::IsWhirlpoolImmune() const
		{
			return m_isActive && m_ult && m_ult->IsWhirlpoolImmune();
		}
	}
}
