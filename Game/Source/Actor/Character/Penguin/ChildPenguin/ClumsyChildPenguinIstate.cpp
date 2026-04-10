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


namespace app
{
	namespace actor
	{
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
		{}


		void ClumsyStandUpState::Exit()
		{
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
		}


		void ClumsySlipState::Update()
		{}


		void ClumsySlipState::Exit()
		{
			/** スリップフラグをリセットする */
			m_owner->SetIsSlipped(false);
		}


		ClumsySlipState::ClumsySlipState(ClumsyChildPenguinStateMachine* owner)
			: ClumsyChildPenguinIState(owner)
		{}
	}
}