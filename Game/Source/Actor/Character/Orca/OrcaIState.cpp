/**
 * @file OrcaIState.cpp
 * @brief シャチのステート
 */
#include "stdafx.h"
#include "OrcaIState.h"
#include "OrcaParameter.h"
#include "OrcaStateMachine.h"
#include "OrcaTypes.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 噛みつきエフェクトの大きさ（EnemyIState.cpp ATTACK_EFFECT_SCALE と同じ） */
			const Vector3 ATTACK_EFFECT_SCALE = { 5.0f, 5.0f, 5.0f };
			/** 気絶中のアニメの再生速度（専用アニメが無いので、ゆっくり泳がせて「止まった」ように見せる） */
			constexpr float STUN_ANIMATION_SPEED = 0.2f;
			/** 噛みつき中に口元へ出す水しぶき（シロクマが水上を走るときの EnEffectKind::SwimSplash と同じもの）の大きさ */
			const Vector3 MOUTH_SPLASH_SCALE = { 20.0f, 18.0f, 20.0f };
			/** 噛む瞬間に出す水しぶきの大きさの倍率（ほかの水しぶきより大きくして、噛んだ手応えを出す） */
			constexpr float IMPACT_SPLASH_SCALE_RATIO = 1.5f;
			/** 口元の水しぶきを出す間隔（EnemyIState.cpp SPLASH_EFFECT_INTERVAL と同じ） */
			constexpr float MOUTH_SPLASH_INTERVAL = 0.2f;
		}


		void OrcaMoveState::Enter()
		{
			m_owner->PlayAnimation(EnOrcaAnimationType::Swim);
		}


		void OrcaMoveState::Update()
		{
			m_owner->MoveInSea();
		}


		/***************************************/


		void OrcaAttackState::Enter()
		{
			m_owner->PlayAnimation(EnOrcaAnimationType::Mouth);
			m_attackTimer = 0.0f;
			m_hasFiredEffect = false;
			m_splashTimer = 0.0f;
			m_owner->SetAttackImpact(false);
			m_owner->SetAttackFinished(false);
		}


		void OrcaAttackState::Update()
		{
			m_owner->MoveInSea();

			const MasterOrcaParameter* parameter = m_owner->GetParameter();
			if (parameter == nullptr) return;

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			m_attackTimer += deltaTime;

			// 浮上しているあいだ、口元に水しぶきを出し続ける（水面の高さに出す）
			m_splashTimer -= deltaTime;
			if (m_splashTimer <= 0.0f)
			{
				m_splashTimer = MOUTH_SPLASH_INTERVAL;
				EffectManager::Get().PlayEffect(
					EnEffectKind::SwimSplash,
					m_owner->GetMouthPosition(),
					Quaternion::Identity,
					MOUTH_SPLASH_SCALE);
			}

			// 噛みついた瞬間にエフェクトとSEを出す（EnemyAttackState::Update と同じ流れ）
			if (!m_hasFiredEffect && m_attackTimer >= parameter->attackImpactTime)
			{
				EffectManager::Get().PlayEffect(
					EnEffectKind::EnemyAttack,
					m_owner->GetMouthPosition(),
					Quaternion::Identity,
					ATTACK_EFFECT_SCALE);
				// 噛んだ瞬間は大きな水しぶきも出す
				EffectManager::Get().PlayEffect(
					EnEffectKind::SwimSplash,
					m_owner->GetMouthPosition(),
					Quaternion::Identity,
					MOUTH_SPLASH_SCALE * IMPACT_SPLASH_SCALE_RATIO);
				SoundManager::Get().PlaySE(enSoundKind_EnemyAttack);
				m_owner->SetAttackImpact(true);
				m_hasFiredEffect = true;
			}

			// 噛んだあと attackHoldTime だけ水面に出たまま止まり、そこで攻撃を終える（終わると沈み始める）。
			// mouth.tka は約3.3秒あるので、アニメの終わりは待たない
			if (m_hasFiredEffect && m_attackTimer >= parameter->attackImpactTime + parameter->attackHoldTime)
			{
				m_owner->SetAttackFinished(true);
			}
		}


		/***************************************/


		void OrcaStunState::Enter()
		{
			m_owner->PlayAnimation(EnOrcaAnimationType::Swim);
			m_owner->SetAnimationSpeed(STUN_ANIMATION_SPEED);
			m_owner->SetMoveDirection(Vector3::Zero);
			m_owner->SetMoveSpeed(0.0f);

			const MasterOrcaParameter* parameter = m_owner->GetParameter();
			m_stunTimer = parameter ? parameter->stunDuration : 0.0f;
		}


		void OrcaStunState::Update()
		{
			// 止まったまま波には追従させる
			m_owner->SetMoveDirection(Vector3::Zero);
			m_owner->MoveInSea();

			m_stunTimer -= g_gameTime->GetFrameDeltaTime();
			if (m_stunTimer <= 0.0f)
			{
				m_owner->SetStun(false);
			}
		}


		void OrcaStunState::Exit()
		{
			m_owner->SetAnimationSpeed(1.0f);
			m_owner->SetStun(false);
		}
	}
}
