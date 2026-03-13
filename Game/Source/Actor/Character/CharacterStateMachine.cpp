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
			m_moveDirection.Normalize();
			const Vector3 moveVector = m_moveDirection * m_moveSpeed;
			const Vector3 nextPosition = m_ownerCharacter->GetCharacterController()->Execute(moveVector, 1 / 60);
			m_ownerActor->SetPosition(nextPosition);

			Quaternion rotation = Quaternion::Identity;
			rotation.SetRotationYFromDirectionXZ(m_moveDirection);
			m_ownerActor->SetRotation(rotation);
		}


		core::IState* CharacterStateMachine::GetChangeState()
		{
			// 何もしない
			return nullptr;
		}


		CharacterStateMachine::CharacterStateMachine(CharacterBase* ownerCharacter, CharacterStatus* characterStatus)
			: ActorStateMachine(ownerCharacter)
			, m_ownerCharacter(ownerCharacter)
			, m_characterStatus(characterStatus)
			, m_moveDirection(Vector3::Zero)
			, m_moveSpeed(0.0f)
			, m_isDash(false)
		{}
	}
}