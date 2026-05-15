/**
 * @file PenguinIState.cpp
 * @brief ペンギン共通のステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinAnimationData.h"
#include "PenguinIState.h"
#include "PenguinStateMachine.h"
#include "PenguinStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const Vector3 SPLASH_EFFECT_SCALE = { 7.0f, 5.0f, 7.0f };

			constexpr float EFFECT_OFFSET_FORWARD = 30.0f;

			constexpr float SPLASH_EFFECT_INTERVAL = 0.2f;// 泳ぎエフェクトの再生間隔

			constexpr float MIN_MOVE_VELOCITY_SQ = 0.1f; // 泳ぎエフェクトを出すための最低速度の二乗

			constexpr float MIN_SPLASH_SCALE_RATIO = 0.5f; // 泳ぎエフェクトの最小スケールの割合（速度が遅いときにエフェクトを小さくするため）

			constexpr float MAX_SPLASH_SCALE_RATIO = 1.0f; // 泳ぎエフェクトの最大スケールの割合（速度が速いときにエフェクトを大きくするため）

			constexpr float MIN_SPEED = 0.1f; // 泳ぎエフェクトのスケールを決めるための最低速度

			constexpr float MAX_SPEED = 1.0f; // 泳ぎエフェクトのスケールを決めるための最高速度

			constexpr float FORWARD_LENGTH_NORMALIZE_SQ = 0.0001f; // 前方ベクトルの正規化を行うかどうかの閾値の二乗

			constexpr float DEFAULT_ANIMATION_SPEED = 1.0f; // アニメーションの再生速度のデフォルト値

			constexpr float SLIDE_END_ANIMATION_SPEED = 2.5f; // スライド終了アニメーションの再生速度

			const Vector3 LANDING_EFFECT_SCALE = { 1.5f, 1.5f, 1.5f }; // 着地エフェクトのスケール

			const Vector3 SLIDE_EFFECT_SCALE = { 1.0f, 1.0f, 1.0f }; // スライドエフェクトのスケール
		}


		PenguinIState::PenguinIState(PenguinStateMachine* owner)
			: m_owner(owner)
			, m_seHandle(INVALID_SE_HANDLE)
		{}




		/************************************/


		void PenguinIdleState::Enter()
		{
			m_owner->SetMoveSpeed(0.0f);
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);
		}


		void PenguinIdleState::Update()
		{
			m_owner->Move();
		}


		void PenguinIdleState::Exit()
		{}


		PenguinIdleState::PenguinIdleState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSneakState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveWalk);

			m_soundHandle = app::INVALID_SE_HANDLE;

			/** 自分が子ペンギンで、かつ可聴対象の場合のみSEを開始する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr
				|| ChildPenguinManager::GetInstance()->IsAudible(child))
			{
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSneak, true);
			}
		}


		void PenguinSneakState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Sneak
			);

			/** 子ペンギンの場合、可聴状態の変化に応じてSEを開始・停止する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr) return;

			const bool isAudible = ChildPenguinManager::GetInstance()->IsAudible(child);
			/** soundHandle が有効値かどうかだけで再生中を判定する */
			/** （FindSE は PlaySE のリクエスト方式により Enter 直後は nullptr を返すため使用しない） */
			const bool isPlaying = (m_soundHandle != app::INVALID_SE_HANDLE);

			if (isAudible && !isPlaying)
			{
				/** 可聴対象になったのでSEを開始する */
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSneak, true);
			}
			else if (!isAudible && isPlaying)
			{
				/** 可聴対象から外れたのでSEを停止する */
				SoundManager::Get().StopSE(m_soundHandle);
				m_soundHandle = app::INVALID_SE_HANDLE;
			}
		}


		void PenguinSneakState::Exit()
		{
			SoundManager::Get().StopSE(m_soundHandle);
			m_soundHandle = app::INVALID_SE_HANDLE;
		}


		PenguinSneakState::PenguinSneakState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinRunState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetRunSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveRun);

			m_soundHandle = INVALID_SE_HANDLE;

			/** 自分が子ペンギンで、かつ可聴対象の場合のみSEを開始する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr
				|| ChildPenguinManager::GetInstance()->IsAudible(child))
			{
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinDash, true);
			}
		}


		void PenguinRunState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Dash
			);

			/** 子ペンギンの場合、可聴状態の変化に応じてSEを開始・停止する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr) return;

			const bool isAudible = ChildPenguinManager::GetInstance()->IsAudible(child);
			/** soundHandle が有効値かどうかだけで再生中を判定する */
			const bool isPlaying = (m_soundHandle != app::INVALID_SE_HANDLE);

			if (isAudible && !isPlaying)
			{
				/** 可聴対象になったのでSEを開始する */
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinDash, true);
			}
			else if (!isAudible && isPlaying)
			{
				/** 可聴対象から外れたのでSEを停止する */
				SoundManager::Get().StopSE(m_soundHandle);
				m_soundHandle = app::INVALID_SE_HANDLE;
			}
		}


		void PenguinRunState::Exit()
		{
			SoundManager::Get().StopSE(m_soundHandle);
			m_soundHandle = app::INVALID_SE_HANDLE;
		}


		PenguinRunState::PenguinRunState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinJumpState::Enter()
		{
			const float jumpPower = m_owner->GetPenguinStatus()->GetJumpPower();
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSneakSpeed();
			m_owner->SetMoveSpeed(moveSpeed);

			// ボタン入力による正規のジャンプ遷移の場合のみJump()を呼ぶ（崖からの落下時は呼ばない）
			if (m_owner->GetIsJump())
			{
				m_owner->SetJumpPower(jumpPower);
				m_owner->Jump();
			}

			m_owner->PlayAnimation(EnPenguinAnimationID::JumpWalking);

			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr || ChildPenguinManager::GetInstance()->IsAudible(child))
			{
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinJump, false);
			}
		}


		void PenguinJumpState::Update()
		{
			m_owner->Move();
		}


		void PenguinJumpState::Exit()
		{
			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Fall
			);

			EffectManager::Get().PlayEffect(
				EnEffectKind::PenguinLanding,
				m_owner->GetTransform().m_position,
				Quaternion::Identity,
				LANDING_EFFECT_SCALE
			);
		}


		PenguinJumpState::PenguinJumpState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideStartState::Enter()
		{
			// 滑るステートと同じ速度を設定
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::SlideStart);
		}


		void PenguinSlideStartState::Update()
		{
			// 移動を可能にする
			m_owner->Move();
		}


		void PenguinSlideStartState::Exit()
		{
			//m_owner->ResetVelocity();
		}


		PenguinSlideStartState::PenguinSlideStartState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlidingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::Sliding);

			m_soundHandle = app::INVALID_SE_HANDLE;

			/** 自分が子ペンギンで、かつ可聴対象の場合のみSEを開始する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr
				|| ChildPenguinManager::GetInstance()->IsAudible(child))
			{
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSlide, true);
			}
		}


		void PenguinSlidingState::Update()
		{
			m_owner->Move();

			/** 足音を出す */
			app::NoiseManager::GetInstance().AddNoise(
				m_owner->GetTransform().m_position,
				app::EnNoiseType::Slide
			);

			EffectManager::Get().PlayEffect(
				EnEffectKind::PenguinSlide,
				m_owner->GetTransform().m_position,
				Quaternion::Identity,
				LANDING_EFFECT_SCALE
			);

			/** 子ペンギンの場合、可聴状態の変化に応じてSEを開始・停止する */
			auto* child = dynamic_cast<ChildPenguin*>(m_owner->GetOwnerPenguinBase());
			if (child == nullptr) return;

			const bool isAudible = ChildPenguinManager::GetInstance()->IsAudible(child);
			/** soundHandle が有効値かどうかだけで再生中を判定する */
			const bool isPlaying = (m_soundHandle != app::INVALID_SE_HANDLE);

			if (isAudible && !isPlaying)
			{
				/** 可聴対象になったのでSEを開始する */
				m_soundHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinSlide, true);
			}
			else if (!isAudible && isPlaying)
			{
				/** 可聴対象から外れたのでSEを停止する */
				SoundManager::Get().StopSE(m_soundHandle);
				m_soundHandle = app::INVALID_SE_HANDLE;
			}
		}


		void PenguinSlidingState::Exit()
		{
			SoundManager::Get().StopSE(m_soundHandle);
			m_soundHandle = app::INVALID_SE_HANDLE;
		}


		PenguinSlidingState::PenguinSlidingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/************************************/


		void PenguinSlideEndState::Enter()
		{
			// 滑るステートと同じ速度を設定
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSlideSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->PlayAnimation(EnPenguinAnimationID::StandUp);
			m_owner->SetAnimationSpeed(SLIDE_END_ANIMATION_SPEED);
		}


		void PenguinSlideEndState::Update()
		{
			// 移動を可能にする
			m_owner->Move();
		}


		void PenguinSlideEndState::Exit()
		{
			m_owner->SetAnimationSpeed(DEFAULT_ANIMATION_SPEED);
		}


		PenguinSlideEndState::PenguinSlideEndState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinSwimmingState::Enter()
		{
			const float moveSpeed = m_owner->GetPenguinStatus()->GetSwimSpeed();
			m_owner->SetMoveSpeed(moveSpeed);
			m_owner->SetIsSwimming(true);
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveSwim);

			if (m_seHandle == app::INVALID_SE_HANDLE)
			{
				m_seHandle = SoundManager::Get().PlaySE(enSoundKind::enSoundKind_PenguinWaterIn, false);
			}

			m_splashEffectTimer = 0.0f;
		}


		void PenguinSwimmingState::Update()
		{
			SoundManager* sound = &SoundManager::Get();
			if (m_seHandle != app::INVALID_SE_HANDLE)
			{
				auto* se = sound->FindSE(m_seHandle);
				if (se && se->IsPlaying()) {
					sound->StopSE(m_seHandle);
					m_seHandle = sound->PlaySE(enSoundKind::enSoundKind_PenguinSwimming, true);
				}
			}

			// 慣性を含む現在の実際の速度を取得
			const Vector3& velocity = m_owner->GetCurrentVelocity();
			float currentSpeed = velocity.Length();

			float maxSpeed = max(MIN_SPEED, m_owner->GetPenguinStatus()->GetSwimSpeed());
			float speedRatio = min(MAX_SPEED, currentSpeed / maxSpeed); // 速度の割合（0.0～1.0）

			float scaleMultiplier = MIN_SPLASH_SCALE_RATIO + ((MAX_SPLASH_SCALE_RATIO - MIN_SPLASH_SCALE_RATIO) * speedRatio);
			Vector3 currentScale = SPLASH_EFFECT_SCALE * scaleMultiplier;

			bool isMoving = (velocity.LengthSq() > MIN_MOVE_VELOCITY_SQ);

			if (isMoving)
			{
				Vector3 effectPosition = m_owner->GetTransform().m_position;
				Quaternion rot = m_owner->GetTransform().m_rotation;
				Vector3 forward = Vector3::AxisZ;

				rot.Apply(forward);
				if (forward.LengthSq() > FORWARD_LENGTH_NORMALIZE_SQ)
				{
					forward.Normalize();
				}

				effectPosition += forward * EFFECT_OFFSET_FORWARD;

				m_splashEffectTimer += g_gameTime->GetFrameDeltaTime();

				// 一定間隔（0.1fなど）ごとに、古いものは消さずに新しいエフェクトを発生させる
				if (m_splashEffectTimer > SPLASH_EFFECT_INTERVAL)
				{
					EffectManager::Get().PlayEffect(
						EnEffectKind::SwimSplash,
						effectPosition,
						Quaternion::Identity,
						currentScale
					);
					m_splashEffectTimer = 0.0f; // タイマーだけリセット
				}
			}
			else
			{
				// 止まっているときも、すでに出た泡は自然に消えるのを待つため、ここでは何もしない
				m_splashEffectTimer = SPLASH_EFFECT_INTERVAL; // 次に動き出した時にすぐ出るようにタイマーを満タンにしておく
			}

			m_owner->Move();
		}


		void PenguinSwimmingState::Exit()
		{
			m_owner->SetIsSwimming(false);
			SoundManager* sound = &SoundManager::Get();

			if (m_seHandle != app::INVALID_SE_HANDLE)
			{
				auto* se = sound->FindSE(m_seHandle);
				if (se && se->IsPlaying()) {
					sound->StopSE(m_seHandle);
					m_seHandle = app::INVALID_SE_HANDLE;
					sound->PlaySE(enSoundKind::enSoundKind_PenguinWaterOut, false);
				}
			}
		}


		PenguinSwimmingState::PenguinSwimmingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
			, m_splashEffectTimer(0.0f)
		{}




		/************************************/


		void PenguinDamagedState::Enter()
		{
			m_owner->Damage();
		}


		void PenguinDamagedState::Update()
		{}


		void PenguinDamagedState::Exit()
		{}


		PenguinDamagedState::PenguinDamagedState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDiyingState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::DeathFaceDown);
		}


		void PenguinDiyingState::Update()
		{}


		void PenguinDiyingState::Exit()
		{}


		PenguinDiyingState::PenguinDiyingState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinDeadState::Enter()
		{
			// 死亡時の処理を派生ステートマシンに委譲する。
			// ChildPenguinStateMachineはここでマネージャーからの削除とdeleteを行う。
			m_owner->OnDead();
		}


		void PenguinDeadState::Update()
		{}


		void PenguinDeadState::Exit()
		{}


		PenguinDeadState::PenguinDeadState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}




		/****************************************/


		void PenguinInWhirlpoolState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveSwim);
		}


		void PenguinInWhirlpoolState::Update()
		{}


		void PenguinInWhirlpoolState::Exit()
		{}


		PenguinInWhirlpoolState::PenguinInWhirlpoolState(PenguinStateMachine* owner)
			: PenguinIState(owner)
		{}
	}
}