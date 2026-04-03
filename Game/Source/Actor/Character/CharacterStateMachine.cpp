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

			// 波面の高さをキャラクターコントローラーに毎フレーム渡す
			// これにより落下処理で波面より下に潜らないようにする
			m_ownerCharacter->GetCharacterController()->SetSeaLevel(waveY);

			// 水中にいる間は重力の切り替えを行わず、波面追従に任せる
			// 水から出た瞬間（前フレームは水中、今フレームは水上）のみ垂直速度をリセットして重力に戻す
			if (!IsInWater())
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(GRAVITY);

				if (m_prevPositionY < waveY)
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

			// 地面コライダーがなく、かつ泳いでいない場合は波面Yに追従させる
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