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
		namespace
		{
			/** 重力の値 */
			constexpr float GRAVITY = -9.8f * 150;
			/** 海面の浮力（重力を反転した値） */
			constexpr float BUOYANCY = -GRAVITY;
		}


		void CharacterStateMachine::Move()
		{
			// 移動方向を正規化して移動ベクトルを計算
			const float deltaTime = g_gameTime->GetFrameDeltaTime();  // 追加
			const Vector3 moveVector = m_moveDirection * m_moveSpeed * deltaTime;  // deltaTime を追加
			Vector3 nextPosition = m_transform.m_position + moveVector;

			const float currentY = m_transform.m_position.y;

			// 海面（y=0）を下回っている場合は浮力（重力反転）を適用して浮かせる
			if (currentY < SEA_LEVEL)
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(BUOYANCY);
			}
			else
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(GRAVITY);

				// 前フレームが水中（y<0）で今フレームが水面以上になった瞬間のみ垂直速度をリセットする
				if (m_prevPositionY < SEA_LEVEL)
				{
					m_ownerCharacter->GetCharacterController()->SetVerticalVelocity(0.0f);
				}
			}

			// 前フレームのY座標を保存
			m_prevPositionY = currentY;

			Vector3 prevPosition = m_ownerCharacter->GetCharacterController()->Execute(nextPosition, 1.0f / 60.0f);
			m_transform.m_position = prevPosition;

			// 移動入力がある場合のみ回転を更新する
			if (m_moveDirection.LengthSq() > FLT_EPSILON)
			{
				Quaternion rotation = m_transform.m_rotation;
				rotation.SetRotationYFromDirectionXZ(m_moveDirection);
				m_transform.m_rotation = rotation;
			}
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
			, m_prevPositionY(0.0f)
		{}
	}
}