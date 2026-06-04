/**
 * @file EnemyIState.cpp
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "EnemyIState.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"
#include "EnemyTypes.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Sound/SoundManager.h"
#include <algorithm>

#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Manager/IglooManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const Vector3 SPLASH_EFFECT_SCALE = { 20.0f, 18.0f, 20.0f };

			const Vector3 ATTACK_EFFECT_SCALE = { 5.0f, 5.0f, 5.0f };

			constexpr float SPLASH_EFFECT_INTERVAL = 0.2f;// 泳ぎエフェクトの再生間隔

			constexpr float MIN_MOVE_VELOCITY_SQ = 0.1f; // 泳ぎエフェクトを出すための最低速度の二乗

			constexpr float MIN_SPLASH_SCALE_RATIO = 0.5f; // 泳ぎエフェクトの最小スケールの割合（速度が遅いときにエフェクトを小さくするため）

			constexpr float MAX_SPLASH_SCALE_RATIO = 1.0f; // 泳ぎエフェクトの最大スケールの割合（速度が速いときにエフェクトを大きくするため）

			const float ATTACK_IMPACT_TIME = 0.5f;

			constexpr float FORWARD_LENGTH_NORMALIZE_SQ = 0.0001f; // 前方ベクトルの正規化を行うかどうかの閾値の二乗

			constexpr float STICK_AMOUNT_THRESHOLD = 0.0001f; // 入力量がこの値以上のときに移動する

			constexpr float STUN_DURATION = 2.0f; // スタンの持続時間

			constexpr int SEARCH_RAND_RATE = 30; // 索敵状態に入るときのランダム率

			constexpr int SAERCE_RAND_MAX = 100; // 索敵状態に入るときのランダム率の最大値

			constexpr float MIN_SPEED = 0.1f; // 泳ぎエフェクトのスケールを決めるための最低速度

			constexpr float MAX_SPEED = 1.0f; // 泳ぎエフェクトのスケールを決めるための最高速度

			constexpr float SEARCH_THRESHOLD = 15.0f; // 索敵に入るための閾値
		}


		EnemyIState::EnemyIState(EnemyStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void EnemyIdleState::Enter()
		{
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Swim);
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Idle);
			}


			SoundManager::Get().PlaySE(enSoundKind_EnemyGrowl);

		}


		void EnemyIdleState::Update()
		{
			// 音の検知処理
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(m_owner->GetPosition(), loudestPos);

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

			m_stunTimer = STUN_DURATION;
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
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Swim);
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::BackWalk);
			}
			if (rand() % SAERCE_RAND_MAX < SEARCH_RAND_RATE)
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

			if (m_stepSE != app::INVALID_SE_HANDLE)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = app::INVALID_SE_HANDLE;
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
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Swim);
				return;
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Walk);
			}

			if (m_stepSE == app::INVALID_SE_HANDLE)
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

			if (m_stepSE == app::INVALID_SE_HANDLE)
			{
				m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);
			}

			// 音の検知処理
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(m_owner->GetPosition(), loudestPos);

			if (totalNoise >= SEARCH_THRESHOLD)
			{
				m_owner->SetSeach(true);
				m_owner->SetSearchTargetPos(loudestPos);
			}
		}


		void EnemyWalkState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != app::INVALID_SE_HANDLE)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = app::INVALID_SE_HANDLE;
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
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Swim);
				return;
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Run);
			}

			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);
			m_owner->SetIsAttack(true);
		}


		void EnemyChaseState::Update()
		{
			if (m_owner->GetStickLAmount() < STICK_AMOUNT_THRESHOLD) {
				return;
			}

			m_owner->Move();
		}


		void EnemyChaseState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			if (m_stepSE != app::INVALID_SE_HANDLE)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = app::INVALID_SE_HANDLE;
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
			m_owner->SetIsSwimming(true);
			m_owner->PlayAnimation(EnEnemyAnimationType::Swim);

			m_splashEffectTimer = 0.0f;
		}


		void EnemySwimState::Update()
		{
			if (m_owner->GetStickLAmount() < STICK_AMOUNT_THRESHOLD)
			{
				return;
			}

			const Vector3& velocity = m_owner->GetCurrentVelocity();
			float currentSpeed = velocity.Length();

			float maxSpeed = max(MIN_SPEED, m_owner->GetStickLAmount());
			float speedRatio = min(MAX_SPEED, currentSpeed / maxSpeed); // 速度の割合（0.0～1.0）

			float scaleMultiplier = MIN_SPLASH_SCALE_RATIO + ((MAX_SPLASH_SCALE_RATIO - MIN_SPLASH_SCALE_RATIO) * speedRatio);
			Vector3 currentScale = SPLASH_EFFECT_SCALE * scaleMultiplier;

			bool isMoving = (velocity.LengthSq() > MIN_MOVE_VELOCITY_SQ);

			if (isMoving)
			{
				Vector3 effectPosition = m_owner->GetPosition();

				m_splashEffectTimer += g_gameTime->GetFrameDeltaTime();

				if (m_splashEffectTimer >= SPLASH_EFFECT_INTERVAL)
				{
					EffectManager::Get().PlayEffect(
						EnEffectKind::SwimSplash,
						effectPosition,
						Quaternion::Identity,
						currentScale
					);
				}
			}
			else
			{
				m_splashEffectTimer = SPLASH_EFFECT_INTERVAL;
			}

			m_owner->Move();
		}


		void EnemySwimState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);
			m_owner->SetIsSwimming(false);
		}


		EnemySwimState::EnemySwimState(EnemyStateMachine* owner)
			: EnemyIState(owner)
			, m_splashEffectTimer(0.0f)
		{

		}




		/************************************/


		void EnemyAttackState::Enter()
		{
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Attack_UnderWater);
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Attack);
			}

			m_attackTimer = 0.0f;
			m_hasFiredEffect = false;
		}


		void EnemyAttackState::Update()
		{
			m_attackTimer += g_gameTime->GetFrameDeltaTime();

			// 指定した時間（振り下ろしの瞬間）が経過し、かつまだ発生させていない場合
			if (!m_hasFiredEffect && m_attackTimer >= ATTACK_IMPACT_TIME)
			{

				Vector3 effectPos = m_owner->GetPosition();

				Quaternion rot = m_owner->GetTransform().m_rotation;
				Vector3 forward = Vector3::AxisZ;
				rot.Apply(forward);
				if (forward.LengthSq() > FORWARD_LENGTH_NORMALIZE_SQ) {
					forward.Normalize();
				}

				// 3. 座標をずらす
				const float OFFSET_FORWARD = 60.0f;
				effectPos += forward * OFFSET_FORWARD;

				const float OFFSET_DOWN = 10.0f;
				effectPos.y += OFFSET_DOWN;


				// エフェクトの再生
				EffectManager::Get().PlayEffect(
					EnEffectKind::EnemyAttack,
					effectPos,
					Quaternion::Identity,
					ATTACK_EFFECT_SCALE
				);

				SoundManager::Get().PlaySE(enSoundKind_EnemyAttack);

				m_owner->SetAttackImpact(true);

				// 発生済みフラグを立てる（何度も呼ばれないようにする）
				m_hasFiredEffect = true;
			}
		}


		void EnemyAttackState::Exit()
		{
			m_owner->SetAttackImpact(false);
			m_owner->SetIsAttack(false);
		}


		EnemyAttackState::EnemyAttackState(EnemyStateMachine* owner)
			: EnemyIState(owner)
		{}




		/************************************/


		void EnemyReturnHomeState::Enter()
		{
			const float moveSpeed = m_owner->GetOwnerStatus()->GetWalkSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			if (m_owner->IsSwim())
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Swim);
				return;
			}
			else
			{
				m_owner->PlayAnimation(EnEnemyAnimationType::Walk);
			}

			m_stepSE = app::SoundManager::Get().PlaySE(enSoundKind_EnemyStep, true);

		}


		void EnemyReturnHomeState::Update()
		{
			if (m_owner->GetStickLAmount() < STICK_AMOUNT_THRESHOLD) {
				return;
			}
			m_owner->Move();
		}


		void EnemyReturnHomeState::Exit()
		{
			m_owner->SetMoveVector(Vector3::Zero);

			if (m_stepSE != app::INVALID_SE_HANDLE)
			{
				app::SoundManager::Get().StopSE(m_stepSE);
				m_stepSE = app::INVALID_SE_HANDLE;
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