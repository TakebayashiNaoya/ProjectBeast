/**
 * @file ClumsyChildPenguinStateMachine.cpp
 * @brief おっちょこちょいペンギンのステートマシン
 */
#include "stdafx.h"
#include "ClumsyChildPenguinIState.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"


namespace app
{
	namespace actor
	{
		ClumsyChildPenguinStateMachine::ClumsyChildPenguinStateMachine(ChildPenguin* ownerChildPenguin)
			: ChildPenguinStateMachine(ownerChildPenguin, EnChildPenguinType::Clumsy)
			, m_ownerChildPenguin(ownerChildPenguin)
		{
			/** おっちょこちょい固有ステートの追加 */
			AddState<ClumsyTripState>(this);
			AddState<ClumsyStandUpState>(this);
			AddState<ClumsySlipState>(this);
		}


		core::IState* ClumsyChildPenguinStateMachine::GetTypeSpecificChangeState()
		{
			/** 転倒・スリップ中のアニメーションが終わったら起き上がりへ */
			if (IsEqualCurrentState(ClumsyTripState::ID()) || IsEqualCurrentState(ClumsySlipState::ID()))
			{
				if (!IsPlayingAnimation())
				{
					return FindState(ClumsyStandUpState::ID());
				}

				/** アニメーション再生中は現在のステートを維持する */
				return IsEqualCurrentState(ClumsyTripState::ID())
					? FindState(ClumsyTripState::ID())
					: FindState(ClumsySlipState::ID());
			}

			/** 起き上がりアニメーション中 */
			if (IsEqualCurrentState(ClumsyStandUpState::ID()))
			{
				/** 世話焼きペンギンに助けてもらった場合はStandUpをスキップして即座にIdleへ戻る */
				if (m_isHelped)
				{
					ChildPenguinManager::GetInstance()->UnregisterDowning(m_ownerChildPenguin);
					m_isHelped = false;
					return nullptr;
				}

				if (!IsPlayingAnimation())
				{
					return nullptr;
				}

				return FindState(ClumsyStandUpState::ID());
			}

			/** スリップ判定：スライド解除時に確率でスリップステートへ */
			if (m_isSlipped)
			{
				return FindState(ClumsySlipState::ID());
			}

			/** 転倒判定：歩き・走り中に確率で転倒ステートへ */
			if (m_isTripped)
			{
				return FindState(ClumsyTripState::ID());
			}

			return nullptr;
		}
	}
}