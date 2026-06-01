/**
 * @file ClumsyChildPenguinIState.cpp
 * @brief おっちょこちょいペンギン固有のステートインターフェース
 * @author 竹林
 */
#include "stdafx.h"
#include "ClumsyChildPenguinIState.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"

#include "graphics/effect/BeastEffectEmitter.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Sound/SoundManager.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 泣きエフェクトの頭部Yオフセット（足元から頭の高さ分） 要調整 */
			constexpr float CRY_EFFECT_HEAD_OFFSET_Y = 80.0f;


			/**
			* @brief 指定座標からカメラを向くビルボード回転を計算する
			*/
			Quaternion CalcBillboardRotation(const Vector3& pos)
			{
				Vector3 toCam = g_camera3D->GetPosition() - pos;
				toCam.Normalize();

				const Vector3 defaultForward(0.0f, 0.0f, 1.0f);
				Vector3 rotAxis;
				rotAxis.Cross(defaultForward, toCam);

				Quaternion result;
				const float lenSq = rotAxis.LengthSq();
				if (lenSq > FLT_EPSILON)
				{
					rotAxis.Normalize();
					const float dot = std::clamp(defaultForward.Dot(toCam), -1.0f, 1.0f);
					result.SetRotation(rotAxis, acosf(dot));
				}
				else if (defaultForward.Dot(toCam) < 0.0f)
				{
					result.SetRotationY(Math::PI);
				}
				return result;
			}
		}


		ClumsyChildPenguinIState::ClumsyChildPenguinIState(ClumsyChildPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/****************************************/


		void ClumsyTripState::Enter()
		{
			m_owner->SetActionInput(Vector3::Zero, false, false, false, false); // 追加
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::Trip);
			ChildPenguinManager::GetInstance()->RegisterDowning(m_owner->GetOwnerChildPenguin());
		}


		void ClumsyTripState::Update()
		{}


		void ClumsyTripState::Exit()
		{
			if (!m_owner->GetIsHelped())
			{
				SoundManager::Get().PlaySE(enSoundKind_ChildPenguinCRY, false, false, enSoundPriority_Hight);
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
			// ── エフェクトが再生中なら毎フレームカメラを向かせる ──
			const EffectHandle handle = m_owner->GetCryEffectHandle();
			if (handle != INVALID_EFFECT_HANDLE)
			{
				auto* emitter = EffectManager::Get().FindEffect(handle);
				if (emitter != nullptr)
				{

				}
				else
				{
					// エフェクト終了済み → ハンドルをリセット
					m_owner->SetCryEffectHandle(INVALID_EFFECT_HANDLE);
				}
			}
		}


		void ClumsyStandUpState::Exit()
		{
			/** 助けられフラグをリセットする */
			m_owner->SetIsHelped(false);

			/** Managerの転倒中リストから解除する */
			ChildPenguinManager::GetInstance()->UnregisterDowning(m_owner->GetOwnerChildPenguin());

			// エフェクトハンドルをリセット
			m_owner->SetCryEffectHandle(INVALID_EFFECT_HANDLE);
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
		}


		void ClumsySlipState::Update()
		{}


		void ClumsySlipState::Exit()
		{
			if (!m_owner->GetIsHelped())
			{
				SoundManager::Get().PlaySE(enSoundKind_ChildPenguinCRY, false, false, enSoundPriority_Hight);

				const Vector3 pos = m_owner->GetOwnerChildPenguin()->GetTransform().m_position;
				//const Vector3 headPos = pos + Vector3(0.0f, CRY_EFFECT_HEAD_OFFSET_Y, 0.0f); // 追加: 頭部オフセット

				// エフェクト再生（ハンドルをステートマシンに保存）
				const EffectHandle handle = EffectManager::Get().PlayEffect(
					EnEffectKind::ChildPenguinCry,
					pos,
					Quaternion::Identity,
					Vector3(6.0f, 6.0f, 6.0f)
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