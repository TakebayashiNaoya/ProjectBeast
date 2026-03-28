/**
 * @file PenguinStateMachine.cpp
 * @brief ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinBase.h"
#include "PenguinStateMachine.h"


namespace app
{
	namespace actor
	{
		void PenguinStateMachine::Jump()
		{
			m_ownerCharacter->GetCharacterController()->Jump(m_jumpPower);
			m_isJump = false; // ジャンプフラグをリセット
		}


		void PenguinStateMachine::Damage()
		{
			// デフォルト実装：各派生クラスでオーバーライド可能
		}


		PenguinStateMachine::PenguinStateMachine(PenguinBase* ownerPenguinBase)
			: CharacterStateMachine(ownerPenguinBase)
			, m_ownerPenguinBase(ownerPenguinBase)
			, m_jumpPower(0.0f)
			, m_isJump(false)
			, m_isSlide(false)
			, m_isDamaged(false)
		{}


		core::IState* PenguinStateMachine::GetChangeState()
		{
			return nullptr;
		}
	}
}