/**
 * @file DaddyPenguinStateMachine.cpp
 * @brief 親ペンギンのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Camera/CameraManager.h"
#include <algorithm> // std::min用


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin)
			: PenguinStateMachine(ownerDaddyPenguin)
			, m_ownerDaddyPenguin(ownerDaddyPenguin)
			, m_isCommandToggle(false)
			, m_isSneak(false)
			, m_isWin(false)
			, m_isLose(false)
		{
			// 共通ステートの追加
			AddState<PenguinIdleState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSneakState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinRunState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinJumpState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlideStartState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlidingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSlideEndState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinSwimmingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDamagedState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDiyingState>(static_cast<PenguinStateMachine*>(this));
			AddState<PenguinDeadState>(static_cast<PenguinStateMachine*>(this));

			// Daddy固有のステートの追加
			AddState<DaddyPenguinCommandShoutState>(this);
			AddState<DaddyPenguinWinState>(this);
			AddState<DaddyPenguinLoseState>(this);

			// 初期ステートの設定
			m_currentState = FindState(PenguinIdleState::ID());
			m_currentState->Enter();
		}


		// =========================================================
		// 入力処理群（分割してスッキリ！）
		// =========================================================

		void DaddyPenguinStateMachine::PlayerControllerInput()
		{
			UpdateMovementInput();
			UpdateActionInput();
			UpdateSystemInput();
		}

		void DaddyPenguinStateMachine::UpdateMovementInput()
		{
			/**
			 * コントローラーの倒し具合で挙動を変えるため、
			 * 正規化前のスティック入力の生データを取ってくる
			 * NOTE: sThumbLXとsThumbLYは-32768～32767の範囲で値が入るため、
			 *		 32767で割って-1.0f～1.0fの範囲に正規化する
			 */
			const XINPUT_STATE& state = g_pad[0]->GetXInputState();
			float rawX = static_cast<float>(state.Gamepad.sThumbLX) / 32767.0f;
			float rawY = static_cast<float>(state.Gamepad.sThumbLY) / 32767.0f;
			float rawLength = sqrtf(rawX * rawX + rawY * rawY);


			/**
			 * コントローラーorエンジンの入力を取る
			 */
			float inputX = 0.0f;			/** X方向の入力 */
			float inputY = 0.0f;			/** Y方向の入力 */
			float stickLength = 0.0f;		/** スティックの倒し具合（0.0f～1.0f） */
			const float DEAD_ZONE = 0.1f;	/** コントローラーの遊び */
			/** ① コントローラーが倒されている場合は生データを採用 */
			if (rawLength > DEAD_ZONE)
			{
				inputX = rawX;
				inputY = rawY;
				stickLength = min(rawLength, 1.0f);
			}
			/** ② コントローラーが触られていない場合はエンジンのデータを採用 */
			else
			{
				inputX = g_pad[0]->GetLStickXF();
				inputY = g_pad[0]->GetLStickYF();
				stickLength = sqrtf(inputX * inputX + inputY * inputY);
				stickLength = min(stickLength, 1.0f);
			}


			/** Bボタンが押されているか（スニークのトグル用） */
			bool isPressB = g_pad[0]->IsPress(enButtonB);


			/** スティックがある程度倒されている場合は移動入力として採用 */
			if (stickLength > 0.1f)
			{
				/** カメラの向きと入力から移動方向を決める */
				const auto& camData = camera::CameraManager::Get().GetCurrentCameraData();
				Vector3 camForward = camData.target - camData.position;
				camForward.y = 0.0f;
				camForward.Normalize();
				Vector3 camRight = { camForward.z, 0.0f, -camForward.x };
				Vector3 inputDir = camRight * inputX + camForward * inputY;
				if (inputDir.LengthSq() > FLT_EPSILON) {
					inputDir.Normalize();
				}
				m_moveDirection = inputDir;

				/** スティックの倒し具合とBボタンの状態でスニークかダッシュかを決める */
				const float SNEAK_THRESHOLD = 0.9f;
				if (stickLength <= SNEAK_THRESHOLD || isPressB) {
					m_isSneak = true;
					SetIsDash(false);
				}
				else {
					m_isSneak = false;
					SetIsDash(true);
				}
			}
			else
			{
				m_moveDirection.Set(Vector3::Zero);
				m_isSneak = false;
				SetIsDash(false);
			}
		}

		void DaddyPenguinStateMachine::UpdateActionInput()
		{
			m_isJump = g_pad[0]->IsTrigger(enButtonA);
			m_isCommandToggle = g_pad[0]->IsTrigger(enButtonY);
			m_isSlide = g_pad[0]->IsPress(enButtonX);
		}

		void DaddyPenguinStateMachine::UpdateSystemInput()
		{
			m_isDamaged = g_pad[0]->IsTrigger(enButtonLB3);
			m_isWin = g_pad[0]->IsTrigger(enButtonLB2);
			m_isLose = g_pad[0]->IsTrigger(enButtonRB2);
		}


		// =========================================================
		// ステータス関連
		// =========================================================

		DaddyPenguinStatus* DaddyPenguinStateMachine::GetDaddyPenguinStatus() const
		{
			return m_ownerActor->GetStatus<DaddyPenguinStatus>();
		}

		const PenguinStatus* DaddyPenguinStateMachine::GetPenguinStatus() const
		{
			return GetDaddyPenguinStatus();
		}

		void DaddyPenguinStateMachine::Damage()
		{
			GetDaddyPenguinStatus()->Damage();
		}


		// =========================================================
		// ステート遷移判定群
		// =========================================================

		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			core::IState* nextState = nullptr;

			// 1. システム・環境系の判定（ダメージ、死亡、水泳など）
			if ((nextState = CheckSystemState()) != nullptr) return nextState;

			// 2. 追従命令・待機命令の判定
			if ((nextState = CheckCommandState()) != nullptr) return nextState;

			// 3. アクション系の判定（ウォーク、ダッシュ、スライド、ジャンプ）
			if ((nextState = CheckActionState()) != nullptr) return nextState;

			// どれにも当てはまらなければ待機状態へ
			return FindState(PenguinIdleState::ID());
		}


		core::IState* DaddyPenguinStateMachine::CheckSystemState()
		{
			if (IsEqualCurrentState(DaddyPenguinWinState::ID())) return FindState(DaddyPenguinWinState::ID());
			if (m_isWin) return FindState(DaddyPenguinWinState::ID());
			if (IsEqualCurrentState(DaddyPenguinLoseState::ID())) return FindState(DaddyPenguinLoseState::ID());
			if (m_isLose) return FindState(DaddyPenguinLoseState::ID());

			/** 1. 死ぬアニメーションが流れていたら死亡中ステートを返し、
					アニメーションが終わったら死亡ステートを返す */
			if (IsEqualCurrentState(PenguinDiyingState::ID())) {
				if (IsPlayingAnimation()) return FindState(PenguinDiyingState::ID());
				else return FindState(PenguinDeadState::ID());
			}

			/** 2. 死亡フラグが立っていたら、死亡させる。 */
			if (m_ownerDaddyPenguin->GetStatus<DaddyPenguinStatus>()->IsDead()) {
				return FindState(PenguinDiyingState::ID());
			}

			/** 3. 被弾フラグが立っていたら被弾ステートへ */
			if (m_isDamaged) {
				return FindState(PenguinDamagedState::ID());
			}

			/** 4. 泳いでいる最中に足が地面についていなかったら泳ぎステートを返し、
					現在は泳いでおらず、泳げる状態なら泳ぎステートを返す */
			if (IsEqualCurrentState(PenguinSwimmingState::ID()))
			{
				if (!IsOnGround()) return FindState(PenguinSwimmingState::ID());
			}
			else if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}

			return nullptr;
		}


		core::IState* DaddyPenguinStateMachine::CheckCommandState()
		{
			if (CanChangeCommandState())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}

			if (IsEqualCurrentState(DaddyPenguinCommandShoutState::ID()) && IsPlayingAnimation())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}

			return nullptr;
		}


		core::IState* DaddyPenguinStateMachine::CheckActionState()
		{
			/** 1. スライド開始ステートのアニメーションが終わっていて、かつスライド状態が続いているならスライドステートへ */
			if (IsEqualCurrentState(PenguinSlideStartState::ID())) {
				if (CanChangeSlidingState()) return FindState(PenguinSlidingState::ID());
			}

			/** 2. スライド終了ステートのアニメーションが終わっていなければスライド終了ステートを続行する */
			if (IsEqualCurrentState(PenguinSlideEndState::ID())) {
				if (!IsFinishedSlideEndState()) return FindState(PenguinSlideEndState::ID());
			}

			/** 3. スライド中で、スライドをキープできるならスライドを維持し、そうでなければスライドを終わらせる */
			if (IsEqualCurrentState(PenguinSlidingState::ID())) {
				if (CanKeepSlidingState()) return FindState(PenguinSlidingState::ID());
				else return FindState(PenguinSlideEndState::ID());
			}

			/** 4. スライドステートに切り替えられるならスライドステートへ */
			if (CanChangeSlideStartState()) {
				return FindState(PenguinSlideStartState::ID());
			}

			/** 5. ジャンプステートに切り替えられるor足が付いていなければジャンプステートへ */
			if (CanChangeJumpState() || !IsOnGround()) {
				return FindState(PenguinJumpState::ID());
			}

			/** 6. 走行ステートに切り替えられるなら走行ステートへ */
			if (CanChangeRunState()) {
				return FindState(PenguinRunState::ID());
			}

			/** 7. 移動入力があるならスニークステートへ */
			if (CanChangeMoveState()) {
				return FindState(PenguinSneakState::ID());
			}

			return nullptr;
		}
	}
}