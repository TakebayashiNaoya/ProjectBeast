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

		namespace
		{
			/** 重力 */
			constexpr float GRAVITY = -9.8f * 5;
			/** 地面(仮) */
			constexpr float GROUND = 0.0f;
		}


		void PenguinStateMachine::Jump()
		{
			// 先に移動処理を行う
			Move();

			// 滞空時間を加算
			m_airTime += g_gameTime->GetFrameDeltaTime();
			// ジャンプパワーと重力から現在のジャンプパワーを計算
			const float jumpPower = m_jumpPower + GRAVITY * m_airTime;

			m_ownerCharacter->GetCharacterController()->Jump(jumpPower);
		}


		void PenguinStateMachine::Damage()
		{
			// デフォルト実装：各派生クラスでオーバーライド可能
		}


		PenguinStateMachine::PenguinStateMachine(PenguinBase* ownerPenguinBase)
			: CharacterStateMachine(ownerPenguinBase)
			, m_ownerPenguinBase(ownerPenguinBase)
			, m_airTime(0.0f)
			, m_jumpPower(0.0f)
			, m_isJump(false)
			, m_isSlide(false)
			, m_isSeparateWater(false)
		{}


		core::IState* PenguinStateMachine::GetChangeState()
		{
			return nullptr;
		}


	}
}