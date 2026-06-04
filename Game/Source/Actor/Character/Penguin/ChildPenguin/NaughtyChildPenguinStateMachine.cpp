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
					return nullptr; // → GetChangeState() 側でIdleが選ばれる
				}
				return FindState(NaughtyWakeBearState::ID()); // アニメ中は維持
			}

			// ── SeekBear 中 ──────────────────────────────────────────────
			if (IsEqualCurrentState(NaughtySeekBearState::ID()))
			{
				if (!m_isGoingToWakeBear)
				{
					// 制止などでフラグが落ちた → 通常に戻す
					return nullptr;
				}
				if (m_isAtBear)
				{
					// 到達 → 起こすステートへ
					return FindState(NaughtyWakeBearState::ID());
				}
				return FindState(NaughtySeekBearState::ID()); // まだ移動中
			}

			// ── 通常ステートからの遷移判定 ───────────────────────────────
			if (m_isGoingToWakeBear)
			{
				return FindState(NaughtySeekBearState::ID());
			}

			return nullptr;
		}
	}
}