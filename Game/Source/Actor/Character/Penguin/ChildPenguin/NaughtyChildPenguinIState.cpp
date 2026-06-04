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

		void NaughtySeekBearState::Enter()
		{
			// AIがBuildInputToTargetで動かすので、ここでは入力を触らない
			// 必要ならManagerに「問題行動中」を登録してもよい
		}

		void NaughtySeekBearState::Update()
		{}

		void NaughtySeekBearState::Exit()
		{}

		NaughtySeekBearState::NaughtySeekBearState(NaughtyChildPenguinStateMachine* owner)
			: NaughtyChildPenguinIState(owner)
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

			const Vector3 pos = m_owner->GetOwnerChildPenguin()->GetTransform().m_position;

			const EffectHandle handle = EffectManager::Get().PlayEffect(
				EnEffectKind::ChildPenguinCry,
				pos,
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

	}
}