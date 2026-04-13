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


namespace app
{
	namespace actor
	{
		DaddyPenguinStateMachine::DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin)
			: PenguinStateMachine(ownerDaddyPenguin)
			, m_ownerDaddyPenguin(ownerDaddyPenguin)
			, m_isCommandToggle(false)
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

			/** 4. Swim中は「水面より完全に出た（IsInWater() == false）」かつ「地面にいる（IsOnGround() == true）」
					の両方を満たすまで維持する。
					これにより、波の下に陸がある波打ち際でのチャタリングを防ぐ。
					現在は泳いでおらず、泳げる状態なら泳ぎステートを返す */
			if (IsEqualCurrentState(PenguinSwimmingState::ID()))
			{
				if (IsInWater() || !IsOnGround()) return FindState(PenguinSwimmingState::ID());
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