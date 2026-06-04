/**
 * @file NaughtyChildPenguinStateMachine.cpp
 * @brief ヤンチャペンギンのステートマシン
 * @author 立山
 */
#include "stdafx.h"
#include "NaughtyChildPenguinIState.h"
#include "NaughtyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"


namespace app
{
	namespace actor
	{
		NaughtyChildPenguinStateMachine::NaughtyChildPenguinStateMachine(ChildPenguin* ownerChildPenguin)
			: ChildPenguinStateMachine(ownerChildPenguin, EnChildPenguinType::Naughty)
			, m_ownerChildPenguin(ownerChildPenguin)
			, m_bearTargetPos(Vector3::Zero)
			, m_targetBear(nullptr)
			, m_isGoingToWakeBear(false)
			, m_isAtBear(false)
		{
			AddState<NaughtySeekBearState>(this);
			AddState<NaughtyWakeBearState>(this);
		}


		core::IState* NaughtyChildPenguinStateMachine::GetTypeSpecificChangeState()
		{
			// ── WakeBear 中 ──────────────────────────────────────────────
			if (IsEqualCurrentState(NaughtyWakeBearState::ID()))
			{
				// アニメ完了 → フラグをリセットして通常へ
				if (!IsPlayingAnimation())
				{
					m_isGoingToWakeBear = false;
					m_isAtBear = false;
					return nullptr; // → GetChangeState() 側でIdle等が選ばれる
				}
				return FindState(NaughtyWakeBearState::ID()); // アニメ中は維持
			}

			// ── シロクマに到達した瞬間 ───────────────────────────────
			if (m_isAtBear)
			{
				return FindState(NaughtyWakeBearState::ID());
			}

			return nullptr;
		}
	}
}