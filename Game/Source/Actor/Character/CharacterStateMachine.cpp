/**
 * @file CharacterStateMachine.cpp
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
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const Vector3 moveVector = m_moveDirection * m_moveSpeed * deltaTime;
			Vector3 nextPosition = m_transform.m_position + moveVector;

			const float currentY = m_transform.m_position.y;

			// 現在のXZ座標における波面Yをバイリニア補間で取得する
			const float waveY = CalcCurrentWaveY();

			// 波面（waveY）を下回っている場合は浮力（重力反転）を適用して浮かせる
			if (currentY < waveY)
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(BUOYANCY);
			}
			else
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(GRAVITY);

				// 前フレームが水中（y < waveY）で今フレームが水面以上になった瞬間のみ垂直速度をリセットする
				if (m_prevPositionY < waveY)
				{
					m_ownerCharacter->GetCharacterController()->SetVerticalVelocity(0.0f);
				}
			}

			// 前フレームのY座標を保存
			m_prevPositionY = currentY;

			Vector3 prevPosition = m_ownerCharacter->GetCharacterController()->Execute(nextPosition, 1.0f / 60.0f);
			m_transform.m_position = prevPosition;

			// ↓ この処理を追加する
			// 地面コライダーがなく、かつ波面付近にいる場合は波面Yに追従させる
			if (!IsOnGround() && !m_isSwimming)
			{
				const float posY = m_transform.m_position.y;
				if (posY < waveY + SEA_SURFACE_THRESHOLD)
				{
					m_transform.m_position.y = waveY;
					m_ownerCharacter->GetCharacterController()->SetPosition(m_transform.m_position);
					m_ownerCharacter->GetCharacterController()->SetVerticalVelocity(0.0f);
				}
			}
		}


		float CharacterStateMachine::CalcCurrentWaveY() const
		{
			const nsBeastEngine::Ocean* ocean = g_renderingEngine->GetOcean();

			// Oceanが未設定の場合は固定の海面高さを返す
			if (ocean == nullptr)
			{
				return SEA_LEVEL;
			}

			const Vector3& pos = m_ownerActor->GetTransform().m_position;

			// コンピュートシェーダーのキャッシュからバイリニア補間した波面Yを返す
			float y = ocean->SampleWaveHeight(pos.x, pos.z);
			return y;
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