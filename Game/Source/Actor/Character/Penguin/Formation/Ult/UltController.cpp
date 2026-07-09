/**
 * @file UltController.cpp
 * @brief ウルトの発動・タイマー・クールダウンを管理するコントローラー
 * @author 竹林
 */
#include "stdafx.h"
#include "UltController.h"
#include "IUltEffect.h"
#include "Source/Actor/Character/Penguin/Formation/Effect/FormationEffectChain.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** ウルト満タンSEの音量倍率 */
			constexpr float ULT_FULL_SE_VOLUME      = 1.0f;
			/** ウルトチャージ中SEの音量倍率 */
			constexpr float ULT_CHARGE_SE_VOLUME    = 1.0f;
			/** ウルト発動SEの音量倍率 */
			constexpr float ULT_ACTIVATE_SE_VOLUME  = 1.0f;
			/** ウルト発動中（ディスチャージ）SEの音量倍率 */
			constexpr float ULT_DISCHARGE_SE_VOLUME = 1.0f;
		}


		UltController::~UltController()
		{
			// 再生中のループSEを止めてから破棄する
			SoundManager::Get().StopSE(m_chargeSeHandle);
			SoundManager::Get().StopSE(m_dischargeSeHandle);
		}


		void UltController::SetUlt(FormationEffectChain* ult, IUltEffect* visual, float duration, float cooldown)
		{
			m_ult       = ult;
			m_ultVisual = visual;
			m_duration  = duration;
			m_cooldown  = cooldown;
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

			// 効果（ロジック）と演出（見た目）の両方に発動を通知する
			m_ult->Enter(ctx);
			if (m_ultVisual) m_ultVisual->Enter(ctx);

			// ウルト発動SEを鳴らす
			SoundManager::Get().PlaySE(enSoundKind_UltActivate, ULT_ACTIVATE_SE_VOLUME);
		}


		void UltController::Update(float dt, const UltContext& ctx)
		{
			// クールダウン更新前の発動可否を控えておき、明けた瞬間を検知する
			const bool wasReady = CanActivate();

			if (m_cooldownTimer > 0.0f)
			{
				m_cooldownTimer -= dt;
			}

			// クールダウンが明けてウルトが発動可能になった瞬間に「満タン」SEを鳴らす
			if (!wasReady && CanActivate())
			{
				SoundManager::Get().PlaySE(enSoundKind_UltFull, ULT_FULL_SE_VOLUME);
			}

			// ウルト発動中はタイマーを進め、終了したらクールダウン（チャージ）を開始する
			if (m_isActive)
			{
				m_timer -= dt;
				m_ult->Update(dt, ctx);
				if (m_ultVisual) m_ultVisual->Update(dt, ctx);

				if (m_timer <= 0.0f)
				{
					m_isActive      = false;
					m_cooldownTimer = m_cooldown;
					m_ult->Exit(ctx);
					if (m_ultVisual) m_ultVisual->Exit(ctx);
				}
			}

			// チャージSE・ディスチャージSEの再生状態を現在の状態に同期する
			UpdateChargeSe();
			UpdateDischargeSe();
		}


		void UltController::UpdateChargeSe()
		{
			// ゲージ蓄積中（ウルト未発動でクールダウン中）はチャージSEをループ再生する。
			// ゲーム開始直後の初回チャージとウルト使用後のチャージのどちらもここで拾う。
			const bool isCharging = (m_ult != nullptr) && !m_isActive && (m_cooldownTimer > 0.0f);
			if (isCharging)
			{
				// まだチャージSEが鳴っていなければ再生を開始する
				if (!SoundManager::Get().IsPlayingSE(m_chargeSeHandle))
				{
					m_chargeSeHandle = SoundManager::Get().PlaySE(enSoundKind_UltCharge, ULT_CHARGE_SE_VOLUME, /*isLoop=*/true);
				}
			}
			else if (m_chargeSeHandle != INVALID_SE_HANDLE)
			{
				// 満タン・発動中などチャージが不要になったらループを止める
				SoundManager::Get().StopSE(m_chargeSeHandle);
				m_chargeSeHandle = INVALID_SE_HANDLE;
			}
		}


		void UltController::UpdateDischargeSe()
		{
			// 発動中（効果持続中）はディスチャージSEをループ再生する
			if (m_isActive)
			{
				// まだディスチャージSEが鳴っていなければ再生を開始する
				if (!SoundManager::Get().IsPlayingSE(m_dischargeSeHandle))
				{
					m_dischargeSeHandle = SoundManager::Get().PlaySE(enSoundKind_UltDischarge, ULT_DISCHARGE_SE_VOLUME, /*isLoop=*/true);
				}
			}
			else if (m_dischargeSeHandle != INVALID_SE_HANDLE)
			{
				// 発動が終わったらループを止める
				SoundManager::Get().StopSE(m_dischargeSeHandle);
				m_dischargeSeHandle = INVALID_SE_HANDLE;
			}
		}


		float UltController::GetCooldownRate() const
		{
			if (m_cooldown <= 0.0f) return 0.0f;
			return max(m_cooldownTimer / m_cooldown, 0.0f);
		}


		float UltController::GetActiveRemainingRate() const
		{
			if (!m_isActive || m_duration <= 0.0f) return 0.0f;
			return max(m_timer / m_duration, 0.0f);
		}
	}
}
