/**
 * @file ClumsyChildPenguinIState.cpp
 * @brief おっちょこちょいペンギン固有のステートインターフェース
 */
#include "stdafx.h"
#include "ClumsyChildPenguinIState.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Sound/SoundManager.h"
#include "graphics/effect/BeastEffectEmitter.h"

#include "Source/Noise/NoiseManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 転んだ時のエフェクトのスケール */
			const Vector3 CRY_EFFECT_SCALE = { 6.0f, 6.0f, 6.0f };
			/** 泣きエフェクトを発生させるボーン名（頭） */
			const wchar_t* CRY_EFFECT_BONE_NAME = L"Head";
		}


		ClumsyChildPenguinIState::ClumsyChildPenguinIState(ClumsyChildPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/****************************************/


		void ClumsyTripState::Enter()
		{
			m_owner->SetActionInput(Vector3::Zero, false, false, false, false);
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::Trip);
			ChildPenguinManager::GetInstance()->RegisterDowning(m_owner->GetOwnerChildPenguin());

			// 復帰までにかかった秒数と介助の有無を出せるように、転倒の開始を控えておく
			m_owner->BeginDown(TimeManager::GetInstance().GetCurTime());

			if (auto* lm = GameLogManager::GetInstance())
				lm->QueueEvent({ {"ev", "clumsy_fall"}, {"penguin_id", m_owner->GetOwnerChildPenguin()->GetLogId()}, {"kind", "trip"} });
		}


		void ClumsyTripState::Update()
		{}


		void ClumsyTripState::Exit()
		{
			if (!m_owner->GetIsHelped())
			{
				SoundManager::Get().PlaySE(enSoundKind_ChildPenguinCRY, 1.0f, false, false, enSoundPriority_Hight);

				const Vector3 pos = m_owner->GetOwnerChildPenguin()->GetTransform().m_position;
				NoiseManager::GetInstance().AddNoise(pos, EnNoiseType::ClumsyCRY, m_owner->GetOwnerChildPenguin()->GetLogId());

				// エフェクトは頭のボーン座標から発生させる
				const Vector3 headPos = m_owner->GetOwnerChildPenguin()->GetModelRender().GetBoneWorldPosition(CRY_EFFECT_BONE_NAME);

				// エフェクト再生（ハンドルをステートマシンに保存）
				const EffectHandle handle = EffectManager::Get().PlayEffect(
					EnEffectKind::ChildPenguinCry,
					headPos,
					Quaternion::Identity,
					CRY_EFFECT_SCALE
				);
				m_owner->SetCryEffectHandle(handle);
			}

			/** 転倒フラグをリセットする */
			m_owner->SetIsTripped(false);
		}


		ClumsyTripState::ClumsyTripState(ClumsyChildPenguinStateMachine* owner)
			: ClumsyChildPenguinIState(owner)
		{}




		/****************************************/


		void ClumsyStandUpState::Enter()
		{
			/** 起き上がるアニメーションを再生する */
			m_owner->PlayAnimation(EnPenguinAnimationID::StandUp);
		}


		void ClumsyStandUpState::Update()
		{
			// 泣きエフェクトの再生中は座標を頭のボーンに追従させる
			const EffectHandle handle = m_owner->GetCryEffectHandle();
			if (handle == INVALID_EFFECT_HANDLE) return;

			auto* effect = EffectManager::Get().FindEffect(handle);
			if (effect == nullptr)
			{
				// エフェクトが自己削除済みの場合はハンドルを無効化する
				m_owner->SetCryEffectHandle(INVALID_EFFECT_HANDLE);
				return;
			}

			const Vector3 headPos = m_owner->GetOwnerChildPenguin()->GetModelRender().GetBoneWorldPosition(CRY_EFFECT_BONE_NAME);
			effect->SetPosition(headPos);
		}


		void ClumsyStandUpState::Exit()
		{
			// 起き上がり完了時にエフェクトを停止する
			const EffectHandle handle = m_owner->GetCryEffectHandle();
			if (handle != INVALID_EFFECT_HANDLE)
			{
				EffectManager::Get().StopEffect(handle);
				m_owner->SetCryEffectHandle(INVALID_EFFECT_HANDLE);
			}

			// 転倒してから立ち上がるまでの秒数を、自力か世話焼きの介助かを添えて記録する。
			// 記録レコードの "t" は整数秒に丸められていて差分を取れないため、秒数はここで計算して持たせる
			// （残り時間は減っていくので、転倒時刻から現在時刻を引いたものが経過秒数になる）
			if (auto* lm = GameLogManager::GetInstance())
			{
				const float downSec = m_owner->GetDownStartTime() - TimeManager::GetInstance().GetCurTime();
				lm->QueueEvent({
					{ "ev",         "clumsy_recover" },
					{ "penguin_id", m_owner->GetOwnerChildPenguin()->GetLogId() },
					{ "sec",        downSec },
					{ "by",         m_owner->GetWasHelpedThisDown() ? "caring" : "self" }
				});
			}

			/** 助けられフラグをリセットする */
			m_owner->SetIsHelped(false);

			/** Managerの転倒中リストから解除する */
			ChildPenguinManager::GetInstance()->UnregisterDowning(m_owner->GetOwnerChildPenguin());
		}


		ClumsyStandUpState::ClumsyStandUpState(ClumsyChildPenguinStateMachine* owner)
			: ClumsyChildPenguinIState(owner)
		{}




		/****************************************/


		void ClumsySlipState::Enter()
		{
			/** 移動入力をゼロにしてこけるアニメーションを再生する */
			/** 速度のリセットはしない（スライドの慣性を残して少し滑らせる） */
			m_owner->SetActionInput(Vector3::Zero, false, false, false, false);
			m_owner->PlayAnimation(EnPenguinAnimationID::Trip);

			/** Managerに転倒中であることを登録する */
			ChildPenguinManager::GetInstance()->RegisterDowning(m_owner->GetOwnerChildPenguin());

			// 復帰までにかかった秒数と介助の有無を出せるように、転倒の開始を控えておく
			m_owner->BeginDown(TimeManager::GetInstance().GetCurTime());

			if (auto* lm = GameLogManager::GetInstance())
				lm->QueueEvent({ {"ev", "clumsy_fall"}, {"penguin_id", m_owner->GetOwnerChildPenguin()->GetLogId()}, {"kind", "slip"} });
		}


		void ClumsySlipState::Update()
		{}


		void ClumsySlipState::Exit()
		{
			if (!m_owner->GetIsHelped())
			{
				SoundManager::Get().PlaySE(enSoundKind_ChildPenguinCRY, 1.0f, false, false, enSoundPriority_Hight);

				const Vector3 pos = m_owner->GetOwnerChildPenguin()->GetTransform().m_position;

				NoiseManager::GetInstance().AddNoise(pos, EnNoiseType::ClumsyCRY, m_owner->GetOwnerChildPenguin()->GetLogId());

				// エフェクトは頭のボーン座標から発生させる
				const Vector3 headPos = m_owner->GetOwnerChildPenguin()->GetModelRender().GetBoneWorldPosition(CRY_EFFECT_BONE_NAME);

				// エフェクト再生（ハンドルをステートマシンに保存）
				const EffectHandle handle = EffectManager::Get().PlayEffect(
					EnEffectKind::ChildPenguinCry,
					headPos,
					Quaternion::Identity,
					CRY_EFFECT_SCALE
				);
				m_owner->SetCryEffectHandle(handle);
			}

			m_owner->SetIsSlipped(false);
		}


		ClumsySlipState::ClumsySlipState(ClumsyChildPenguinStateMachine* owner)
			: ClumsyChildPenguinIState(owner)
		{}
	}
}