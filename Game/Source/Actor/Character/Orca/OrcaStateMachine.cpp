/**
 * @file OrcaStateMachine.cpp
 * @brief シャチのステートマシン
 */
#include "stdafx.h"
#include "OrcaStateMachine.h"
#include "Orca.h"
#include "OrcaIState.h"
#include "OrcaParameter.h"
#include "OrcaSeaGrid.h"
#include "OrcaStatus.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 角度を −π〜π に収める */
			float WrapAngle(float angle)
			{
				while (angle > Math::PI) angle -= Math::PI * 2.0f;
				while (angle < -Math::PI) angle += Math::PI * 2.0f;
				return angle;
			}

			/**
			 * 曲がっている途中でも出す速さの下限（目標の速さに対する割合）
			 * @details 0 にすると真後ろを向くまでその場で回るだけになり、シャチらしい弧を描かない
			 */
			constexpr float MIN_TURNING_SPEED_RATIO = 0.2f;
		}


		OrcaStateMachine::OrcaStateMachine(Orca* owner)
			: CharacterStateMachine(owner)
			, m_owner(owner)
		{
			AddState<OrcaMoveState>(this);
			AddState<OrcaAttackState>(this);
			AddState<OrcaStunState>(this);

			m_currentState = FindState(OrcaMoveState::ID());
		}


		OrcaStateMachine::~OrcaStateMachine()
		{}


		core::IState* OrcaStateMachine::GetChangeState()
		{
			// 優先順は EnemyStateMachine::GetChangeState（:102-138）と同じ: 気絶 > 攻撃 > 移動
			// （シャチは眠らないので睡眠の維持は無い。咆哮は「止まって向きを変える」だけなので移動ステートで行う）
			if (m_isStun)   return FindState(OrcaStunState::ID());
			if (m_isAttack) return FindState(OrcaAttackState::ID());
			return FindState(OrcaMoveState::ID());
		}


		const MasterOrcaParameter* OrcaStateMachine::GetParameter() const
		{
			const OrcaStatus* status = GetOwnerStatus();
			return status ? status->GetParameter() : nullptr;
		}


		OrcaStatus* OrcaStateMachine::GetOwnerStatus() const
		{
			return m_owner ? m_owner->GetStatus<OrcaStatus>() : nullptr;
		}


		void OrcaStateMachine::MoveInSea()
		{
			const MasterOrcaParameter* parameter = GetParameter();
			if (parameter == nullptr) return;

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// --- 大きさ ---
			m_transform.m_scale = Vector3(parameter->modelScale, parameter->modelScale, parameter->modelScale);

			// --- 向きと速さ ---
			float targetSpeed = 0.0f;
			if (m_moveDirection.LengthSq() > FLT_EPSILON)
			{
				const float desiredYaw = atan2f(m_moveDirection.x, m_moveDirection.z);
				const float diff = WrapAngle(desiredYaw - m_yaw);
				const float maxTurn = parameter->turnSpeedDeg * (Math::PI / 180.0f) * m_turnSpeedMultiplier * deltaTime;
				const float turn = min(max(diff, -maxTurn), maxTurn);
				m_yaw = WrapAngle(m_yaw + turn);

				// 目標の向きとの角度が大きいほど減速して、狭い水路でも曲がり切れるようにする
				const float remaining = min(fabsf(diff - turn), Math::PI * 0.5f);
				const float ratio = max(MIN_TURNING_SPEED_RATIO, cosf(remaining));
				targetSpeed = m_moveSpeed * m_speedMultiplier * ratio;
			}
			const float lerpFactor = min(1.0f, parameter->acceleration * deltaTime);
			m_currentSpeed += (targetSpeed - m_currentSpeed) * lerpFactor;

			// --- 位置（XZ は必ず ResolveMove の戻り値で書き換える） ---
			const Vector3 forward = GetForward();
			Vector3 next = m_transform.m_position + forward * (m_currentSpeed * deltaTime);
			if (m_seaGrid != nullptr && m_seaGrid->IsBuilt())
			{
				const Vector3 resolved = m_seaGrid->ResolveMove(m_transform.m_position, next);
				m_transform.m_position.x = resolved.x;
				m_transform.m_position.z = resolved.z;
			}
			else
			{
				m_transform.m_position.x = next.x;
				m_transform.m_position.z = next.z;
			}
			// --- 深さと頭の持ち上げ（噛みつくときだけ浮上して上を向き、口を水面に出す） ---
			UpdateDepthAndPitch(deltaTime);
			ApplyRotation();

			// --- 高さ（Y = 波面 − 深さ） ---
			const float waveY = CalcCurrentWaveY();
			m_transform.m_position.y = waveY - m_depth;

		}


		void OrcaStateMachine::ApplyKnockback(const Vector3& direction, float distance)
		{
			Vector3 flat(direction.x, 0.0f, direction.z);
			if (flat.LengthSq() <= FLT_EPSILON) return;
			flat.Normalize();

			const Vector3 target = m_transform.m_position + flat * distance;
			if (m_seaGrid != nullptr && m_seaGrid->IsBuilt())
			{
				const Vector3 resolved = m_seaGrid->ResolveMove(m_transform.m_position, target);
				m_transform.m_position.x = resolved.x;
				m_transform.m_position.z = resolved.z;
			}
			m_currentSpeed = 0.0f;
		}


		void OrcaStateMachine::SetFacing(const Vector3& direction)
		{
			if (direction.x * direction.x + direction.z * direction.z <= FLT_EPSILON) return;
			m_yaw = atan2f(direction.x, direction.z);
			ApplyRotation();
		}


		void OrcaStateMachine::InitializePose()
		{
			const MasterOrcaParameter* parameter = GetParameter();
			if (parameter == nullptr) return;

			m_transform.m_scale = Vector3(parameter->modelScale, parameter->modelScale, parameter->modelScale);
			m_isDepthInitialized = false;
			UpdateDepthAndPitch(0.0f);
			m_transform.m_position.y = CalcCurrentWaveY() - m_depth;
			ApplyRotation();
		}


		void OrcaStateMachine::ApplyRotation()
		{
			Quaternion yawRotation;
			yawRotation.SetRotationY(m_yaw);
			if (fabsf(m_pitch) <= FLT_EPSILON)
			{
				m_transform.m_rotation = yawRotation;
				return;
			}

			// 正面を上へ傾ける回転（ワールド空間）を、左右の向きの後に掛ける（Enemy::UpdateGroundTilt と同じ掛け方）
			const Vector3 forward = GetForward();
			Vector3 pitched = forward * cosf(m_pitch) + Vector3::Up * sinf(m_pitch);
			pitched.Normalize();
			Quaternion pitchRotation;
			pitchRotation.SetRotation(forward, pitched);
			m_transform.m_rotation.Multiply(yawRotation, pitchRotation);
		}


		Vector3 OrcaStateMachine::GetForward() const
		{
			return Vector3(sinf(m_yaw), 0.0f, cosf(m_yaw));
		}


		Vector3 OrcaStateMachine::GetSurfacePosition() const
		{
			return Vector3(m_transform.m_position.x, CalcCurrentWaveY(), m_transform.m_position.z);
		}


		Vector3 OrcaStateMachine::GetMouthPosition() const
		{
			const MasterOrcaParameter* parameter = GetParameter();
			const float offset = parameter ? parameter->mouthOffset : 0.0f;
			return GetSurfacePosition() + GetForward() * offset;
		}


		void OrcaStateMachine::UpdateDepthAndPitch(float deltaTime)
		{
			const MasterOrcaParameter* parameter = GetParameter();
			if (parameter == nullptr) return;

			if (!m_isDepthInitialized)
			{
				m_depth = GetTargetDepth();
				m_pitch = GetTargetPitch();
				m_transitionLevel = m_depthLevel;
				m_transitionElapsed = 0.0f;
				m_transitionDuration = 0.0f;
				m_isDepthInitialized = true;
				return;
			}

			// 段階が変わったら、いまの値から新しい段階の値への移り変わりを始める
			if (m_depthLevel != m_transitionLevel)
			{
				const EnOrcaDepthLevel previous = m_transitionLevel;
				m_transitionLevel = m_depthLevel;
				m_depthFrom = m_depth;
				m_pitchFrom = m_pitch;
				m_transitionElapsed = 0.0f;

				if (m_depthLevel == EnOrcaDepthLevel::Attack)
				{
					// 噛みつき: 浮上して頭を持ち上げきるのが噛む瞬間（attackImpactTime）に合うようにする
					m_transitionDuration = parameter->attackRiseTime;
				}
				else if (previous == EnOrcaDepthLevel::Attack)
				{
					m_transitionDuration = parameter->attackSinkTime;
				}
			}

			const float depthTo = GetTargetDepth();
			const float pitchTo = GetTargetPitch();
			m_transitionElapsed += deltaTime;
			if (m_transitionDuration <= FLT_EPSILON || m_transitionElapsed >= m_transitionDuration)
			{
				m_depth = depthTo;
				m_pitch = pitchTo;
				return;
			}

			// イーズインアウト（smoothstep）: ゆっくり動き出し、ゆっくり止まる
			const float t = m_transitionElapsed / m_transitionDuration;
			const float eased = t * t * (3.0f - 2.0f * t);
			m_depth = m_depthFrom + (depthTo - m_depthFrom) * eased;
			m_pitch = m_pitchFrom + (pitchTo - m_pitchFrom) * eased;
		}


		float OrcaStateMachine::GetTargetPitch() const
		{
			const MasterOrcaParameter* parameter = GetParameter();
			if (parameter == nullptr || m_depthLevel != EnOrcaDepthLevel::Attack) return 0.0f;
			return parameter->attackPitchDeg * (Math::PI / 180.0f);
		}


		float OrcaStateMachine::GetTargetDepth() const
		{
			const MasterOrcaParameter* parameter = GetParameter();
			if (parameter == nullptr) return 0.0f;
			switch (m_depthLevel)
			{
			case EnOrcaDepthLevel::Attack: return parameter->attackDepth;
			default:                       return parameter->swimDepth;
			}
		}


		const char* OrcaStateMachine::GetStateNameForLog() const
		{
			return m_stateNameForLog;
		}
	}
}
