/**
 * @file CharacterStateMachine.cpp
 * @brief キャラクターのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterStateMachine.h"
#include "Source/Nature/Ocean.h"


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
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// Lerpの割合の最大値を定数化してマジックナンバーを防ぐ
			constexpr float MAX_LERP_FACTOR = 1.0f;

			// --- 慣性と摩擦の計算 ---
			if (m_moveDirection.LengthSq() > FLT_EPSILON)
			{
				Vector3 targetVelocity = m_moveDirection * (m_moveSpeed * m_speedMultiplier);

				// ★修正：1.0f の代わりに MAX_LERP_FACTOR を使う
				float lerpFactor = min(MAX_LERP_FACTOR, m_acceleration * deltaTime);

				m_currentVelocity.Lerp(lerpFactor, m_currentVelocity, targetVelocity);
			}
			else
			{
				// ★修正：こちらも同様
				float lerpFactor = min(MAX_LERP_FACTOR, m_friction * deltaTime);

				m_currentVelocity.Lerp(lerpFactor, m_currentVelocity, Vector3::Zero);
			}

			// 実際の移動量を計算
			const Vector3 moveVector = m_currentVelocity * deltaTime;
			Vector3 nextPosition = m_transform.m_position + moveVector;

			// --- 波面処理・重力切り替え（既存のまま） ---
			const float currentY = m_transform.m_position.y;
			const float waveY = CalcCurrentWaveY();

			m_ownerCharacter->GetCharacterController()->SetSeaLevel(waveY);

			if (!IsInWater())
			{
				m_ownerCharacter->GetCharacterController()->SetGravity(GRAVITY);
				if (m_prevPositionY < waveY)
				{
					m_ownerCharacter->GetCharacterController()->SetVerticalVelocity(0.0f);
				}
			}
			m_prevPositionY = currentY;

			// --- キャラクターコントローラーによるコリジョン解決 ---
			Vector3 prevPosition = m_ownerCharacter->GetCharacterController()->Execute(nextPosition, 1.0f / 60.0f);
			m_transform.m_position = prevPosition;

			// --- Slerpを用いた滑らかな回転 ---

			// ★追加：回転を許可する速度のしきい値（マジックナンバーを排除）
			constexpr float ROTATE_VELOCITY_THRESHOLD_SQ = 0.1f;


			if (m_currentVelocity.LengthSq() > ROTATE_VELOCITY_THRESHOLD_SQ) // 微小な押し出しでガタつかないための閾値
			{
				Vector3 velocityDir = m_currentVelocity;
				velocityDir.y = 0.0f; // Y軸は無視する
				velocityDir.Normalize();

				// 今進んでいる方向（速度ベクトル）を目標の回転とする
				Quaternion targetRotation = m_transform.m_rotation;
				targetRotation.SetRotationYFromDirectionXZ(velocityDir);

				// ★修正：Slerpの割合も、先ほど作った MAX_LERP_FACTOR (1.0f) を超えないように制限する
				float slerpFactor = min(MAX_LERP_FACTOR, m_turnSpeed * deltaTime);

				// 現在の回転から目標の回転へ Slerp で徐々に補間する（滑らかに振り向く）
				m_transform.m_rotation.Slerp(slerpFactor, m_transform.m_rotation, targetRotation);
			}

			// --- 既存の波面追従処理 ---
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
			const nature::Ocean* ocean = nature::Ocean::GetInstance();

			// Oceanが未設定の場合は固定の海面高さを返す
			if (ocean == nullptr)
			{
				return SEA_LEVEL;
			}

			const Vector3& pos = m_ownerActor->GetTransform().m_position;

			// コンピュートシェーダーのキャッシュからバイリニア補間した波面Yを返す
			return ocean->SampleWaveHeight(pos.x, pos.z);
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
			, m_speedMultiplier(1.0f)
			, m_isDash(false)
			, m_isSwimming(false)
			, m_prevPositionY(0.0f)
			, m_currentVelocity(Vector3::Zero)
			, m_acceleration(10.0f)
			, m_friction(5.0f)
			, m_turnSpeed(8.0f)
		{}
	}
}