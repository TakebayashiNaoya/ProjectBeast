/**
 * @file EnemyIState.cpp
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyController.h"
#include "EnemyIState.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"
#include "EnemyTypes.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Sound/SoundManager.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		EnemyIState::EnemyIState(EnemyStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void EnemyIdleState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Idle);


			SoundManager::Get().PlaySE(enSoundKind_EnemyGrowl);

		}


		void EnemyIdleState::Update()
		{
			// 音の検知処理
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(m_owner->GetPosition(), loudestPos);
			const float SEARCH_THRESHOLD = 15.0f; // 索敵に入るための閾値

			if (totalNoise >= SEARCH_THRESHOLD)
			{
				m_owner->SetSeach(true);
				m_owner->SetSearchTargetPos(loudestPos);
			}
		}


		void EnemyIdleState::Exit()
		{}


		EnemyIdleState::EnemyIdleState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/




		void EnemyStunState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Stun);
			m_owner->SetStickLAmount(0.0f); // 動かない

			m_stunTimer = 2.0f;
		}


		void EnemyStunState::Update()
		{
			m_stunTimer -= g_gameTime->GetFrameDeltaTime();

			if (!m_owner->IsPlayingAnimation() || m_stunTimer <= 0.0f)
			{
				m_owner->SetStun(false); // ←ここで解除！
			}
		}

		void EnemyStunState::Exit()
		{
			m_owner->SetStun(false);
		}

		EnemyStunState::EnemyStunState(EnemyStateMachine* owner)
			:EnemyIState(owner),
			m_stunTimer(0.0f)
		{

		}


		/************************************/



		void EnemySearchState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::BackWalk);
			if (rand() % 100 < 30)
			{
				SoundManager::Get().PlaySE(enSoundKind_EnemyGrowl);
			}
			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemySearchState::Update()
		{
			m_owner->Move();
		}


		void EnemySearchState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemySearchState::EnemySearchState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyWalkState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Walk);

			if (m_stepSE == -1)
			{
				m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true, false);
			}
		}


		void EnemyWalkState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}
			m_owner->Move();

			if (m_stepSE == -1)
			{
				m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);
			}

			// 音の検知処理
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(m_owner->GetPosition(), loudestPos);
			const float SEARCH_THRESHOLD = 15.0f; // 索敵に入るための閾値

			if (totalNoise >= SEARCH_THRESHOLD)
			{
				m_owner->SetSeach(true);
				m_owner->SetSearchTargetPos(loudestPos);
			}
		}


		void EnemyWalkState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyWalkState::EnemyWalkState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyChaseState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Run);


			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemyChaseState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}

			m_owner->Move();
		}


		void EnemyChaseState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyChaseState::EnemyChaseState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyJumpState::Enter()
		{

		}


		void EnemyJumpState::Update()
		{

		}


		void EnemyJumpState::Exit()
		{

		}


		EnemyJumpState::EnemyJumpState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{

		}




		/************************************/


		void EnemySwimState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Run);
		}


		void EnemySwimState::Update()
		{
			m_owner->Move();
		}


		void EnemySwimState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}

		EnemySwimState::EnemySwimState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{

		}




		/************************************/


		void EnemyAttackState::Enter()
		{
			m_owner->PlayAnimation(EnEnemyAnimationType::Attack);
		}


		void EnemyAttackState::Update()
		{

		}


		void EnemyAttackState::Exit()
		{}


		EnemyAttackState::EnemyAttackState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyReturnHomeState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnEnemyAnimationType::Walk);

			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemyReturnHomeState::Update()
		{
			if (m_owner->GetStickLAmount() < 0.0001f) {
				return;
			}
			m_owner->Move();
		}


		void EnemyReturnHomeState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != -1)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = -1;
			}
		}


		EnemyReturnHomeState::EnemyReturnHomeState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyCoolDownState::Enter()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			m_owner->PlayAnimation(EnEnemyAnimationType::Sleep);
			SoundManager::Get().PlaySE(enSoundKind_EnemyRoar);

			// 寝た瞬間に両ゲージを満タンにする
			m_owner->SetWakeUpGauge(MAX_WAKE_UP_GAUGE);
			m_owner->SetSleepTimer(MAX_SLEEP_TIME);
		}


		void EnemyCoolDownState::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			//------------------------------------------------------------
			// 音ゲージの更新
			//------------------------------------------------------------
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(m_owner->GetPosition(), loudestPos);

			float wakeUpGauge = m_owner->GetWakeUpGauge();

			if (totalNoise > 0.0f)
			{
				// 騒がしいほどゲージが減る
				wakeUpGauge -= totalNoise * deltaTime;
			}
			else
			{
				// 静かなときはゲージが回復する
				wakeUpGauge += GAUGE_RECOVERY_SPEED * deltaTime;
			}

			wakeUpGauge = std::clamp(wakeUpGauge, 0.0f, MAX_WAKE_UP_GAUGE);
			m_owner->SetWakeUpGauge(wakeUpGauge);

			//------------------------------------------------------------
			// 睡眠タイマーの更新（回復なし、減り続けるのみ）
			//------------------------------------------------------------
			float sleepTimer = m_owner->GetSleepTimer();
			sleepTimer -= deltaTime;
			sleepTimer = max(0.0f, sleepTimer);
			m_owner->SetSleepTimer(sleepTimer);

			//------------------------------------------------------------
			// 起床判定（OR条件：どちらかが0になれば起きる）
			//------------------------------------------------------------
			if (wakeUpGauge <= 0.0f || sleepTimer <= 0.0f)
			{
				m_owner->SetCoolDown(false);

				// 音で起きた場合は索敵状態へ
				if (wakeUpGauge <= 0.0f)
				{
					m_owner->SetSeach(true);
					m_owner->SetSearchTargetPos(loudestPos);
				}
			}
		}


		void EnemyCoolDownState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
		}


		EnemyCoolDownState::EnemyCoolDownState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyRoarState::Enter()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			m_owner->PlayAnimation(EnEnemyAnimationType::Buff);

		}


		void EnemyRoarState::Update()
		{

		}


		void EnemyRoarState::Exit()
		{
			if (m_owner->IsPlayingAnimation())return;
		}


		EnemyRoarState::EnemyRoarState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{

		}
	}
}