/**
 * @file NaughtyChildPenguinIState.cpp
 * @brief ヤンチャペンギン固有のステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "NaughtyChildPenguinIState.h"
#include "NaughtyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Noise/NoiseManager.h" 
#include "Source/Sound/SoundManager.h"

namespace app {
	namespace actor {

		namespace
		{
			const Vector3 EFFECT_SCALE = { 6.0f, 6.0f, 6.0f };
		}

		NaughtyChildPenguinIState::NaughtyChildPenguinIState(NaughtyChildPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/****************************************/


		void NaughtyWakeBearState::Enter()
		{
			m_owner->SetActionInput(Vector3::Zero, false, false, false, false);
			m_owner->SetMoveSpeed(0.0f);

			// TODO: シロクマを起こすアニメID に差し替える
			m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);

			// 効果音（やんちゃな声など）
			SoundManager::Get().PlaySE(enSoundKind_NaughtyPoke, false, false, enSoundPriority_Hight);

			const Vector3& myPos = m_owner->GetOwnerChildPenguin()->GetTransform().m_position;
			NoiseManager::GetInstance().AddNoise(myPos, EnNoiseType::NaughtyPoke);
			EffectManager::Get().PlayEffect(
				EnEffectKind::ChildPenguinCry,
				myPos,
				Quaternion::Identity,
				EFFECT_SCALE
			);
		}

		void NaughtyWakeBearState::Update()
		{}

		void NaughtyWakeBearState::Exit()
		{

		}

		NaughtyWakeBearState::NaughtyWakeBearState(NaughtyChildPenguinStateMachine* owner)
			: NaughtyChildPenguinIState(owner)
		{}




		/****************************************/

		// ★ 追加：渦潮に飛び込むステートの実装
		void NaughtyDiveWhirlpoolState::Enter()
		{
			m_owner->SetActionInput(Vector3::Zero, false, false, false, false);
			m_owner->SetMoveSpeed(0.0f);

			m_owner->PlayAnimation(EnPenguinAnimationID::LaunchBegin);

			// 歓声などの効果音
			SoundManager::Get().PlaySE(enSoundKind_ChildPenguinCRY, false, false, enSoundPriority_Hight);
		}

		void NaughtyDiveWhirlpoolState::Update()
		{}

		void NaughtyDiveWhirlpoolState::Exit()
		{}

		NaughtyDiveWhirlpoolState::NaughtyDiveWhirlpoolState(NaughtyChildPenguinStateMachine* owner)
			: NaughtyChildPenguinIState(owner)
		{}
	}
}