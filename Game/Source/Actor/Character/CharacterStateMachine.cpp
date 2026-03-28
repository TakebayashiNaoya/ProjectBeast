/**
 * @file CharacterStateMachine.h
 * @brief キャラクターのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterStateMachine.h"


namespace app
{
	namespace actor
	{
		void CharacterStateMachine::Move()
		{
			// 移動方向を正規化して移動ベクトルを計算
			const Vector3 moveVector = m_moveDirection * m_moveSpeed;
			Vector3 nextPosition = m_transform.m_position + moveVector;
			Vector3 prevPosition = m_ownerCharacter->GetCharacterController()->Execute(nextPosition, 1.0f / 60.0f);
			m_transform.m_position = prevPosition;

			Quaternion rotation = m_transform.m_rotation;
			rotation.SetRotationYFromDirectionXZ(m_moveDirection);
			m_transform.m_rotation = rotation;
		}


		core::IState* CharacterStateMachine::GetChangeState()
		{
			// 何もしない
			return nullptr;
		}


		CharacterStateMachine::CharacterStateMachine(CharacterBase* ownerCharacter)
			: ActorStateMachine(ownerCharacter)
			, m_ownerCharacter(ownerCharacter)
			, m_moveDirection(Vector3::Zero)
			, m_moveSpeed(0.0f)
			, m_isDash(false)
		{}
	}
}