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


		void DaddyPenguinStateMachine::PlayerControllerInput()
		{
			float stickX = g_pad[0]->GetLStickXF();
			float stickY = g_pad[0]->GetLStickYF(); // Yは奥方向の入力

			// カメラの向きを考慮した移動ベクトルの計算
			const auto& camData = camera::CameraManager::Get().GetCurrentCameraData();

			// カメラの前方向ベクトルと右方向ベクトルを計算
			Vector3 camForward = camData.target - camData.position;
			camForward.y = 0.0f;      // 水平方向のみにする（空や地面に向かって移動しないように）
			camForward.Normalize();   // 長さを1にする

			Vector3 camRight = { camForward.z, 0.0f, -camForward.x }; // Y軸回りに90度回転させて右ベクトルを作成

			// カメラの向きを基準に移動ベクトルを算出
			m_moveDirection = camRight * stickX + camForward * stickY;

			// スティックの入力強度（ベクトル長）を計算
			float stickLength = sqrtf(stickX * stickX + stickY * stickY);
			bool isPressB = g_pad[0]->IsPress(enButtonB);

			// 遊び(デッドゾーン)を考慮
			if (stickLength > 0.1f)
			{
				// スティック50%以下、またはBボタンが押されている場合はスニーク
				if (stickLength <= 0.5f || isPressB)
				{
					m_isSneak = true;
					m_isDash = false;
				}
				// それ以外（スティック50%より大きく、Bボタンなし）はダッシュ
				else
				{
					m_isSneak = false;
					m_isDash = true;
				}
			}
			else
			{
				m_isSneak = false;
				m_isDash = false;
			}

			m_isJump = g_pad[0]->IsTrigger(enButtonA);
			m_isSlide = g_pad[0]->IsPress(enButtonX);

			m_isCommandToggle = g_pad[0]->IsTrigger(enButtonY);

			m_isDamaged = g_pad[0]->IsTrigger(enButtonLB3);

			m_isWin = g_pad[0]->IsTrigger(enButtonLB2);
			m_isLose = g_pad[0]->IsTrigger(enButtonRB2);
		}


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


		core::IState* DaddyPenguinStateMachine::GetChangeState()
		{
			/** 命令中なら命令ステートへ */
			if (CanChangeCommandState())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}

			if (IsEqualCurrentState(DaddyPenguinWinState::ID()))
			{
				return FindState(DaddyPenguinWinState::ID());
			}

			if (m_isWin)
			{
				return FindState(DaddyPenguinWinState::ID());
			}

			if (IsEqualCurrentState(DaddyPenguinLoseState::ID()))
			{
				return FindState(DaddyPenguinLoseState::ID());
			}

			if (m_isLose)
			{
				return FindState(DaddyPenguinLoseState::ID());
			}

			/** 死亡ステート中、アニメーション再生中であれば継続 */
			if (IsEqualCurrentState(PenguinDiyingState::ID()))
			{
				if (IsPlayingAnimation())
				{
					return FindState(PenguinDiyingState::ID());
				}
				else
				{
					return FindState(PenguinDeadState::ID());
				}
			}

			if (m_ownerDaddyPenguin->GetStatus<DaddyPenguinStatus>()->IsDead())
			{
				return FindState(PenguinDiyingState::ID());
			}

			if (m_isDamaged)
			{
				return FindState(PenguinDamagedState::ID());
			}

			/** 泳ぐステートの維持・変更 */
			if (IsEqualCurrentState(PenguinSwimmingState::ID()))
			{
				if (!IsOnGround())
				{
					return FindState(PenguinSwimmingState::ID());
				}
			}
			else if (CanChangeSwimState())
			{
				return FindState(PenguinSwimmingState::ID());
			}

			/** ジャンプ開始、または滞空（落下中）状態の維持 */
			// ※水泳条件に当てはまらず、空中にいる場合は常にジャンプステートにする
			if (CanChangeJumpState() || !IsOnGround())
			{
				return FindState(PenguinJumpState::ID());
			}

			/** 命令中なら維持する */
			if (IsEqualCurrentState(DaddyPenguinCommandShoutState::ID())
				&& IsPlayingAnimation())
			{
				return FindState(DaddyPenguinCommandShoutState::ID());
			}

			/** スライド開始中ならアニメーションが終わるまで維持し、終わるとスライディングステートへ */
			if (IsEqualCurrentState(PenguinSlideStartState::ID()))
			{
				if (CanChangeSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
			}

			/** スライド終わりのアニメーション再生中なら維持する */
			if (IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				if (!IsFinishedSlideEndState())
				{
					return FindState(PenguinSlideEndState::ID());
				}
			}

			/** スライド中なら、スライドを維持するか判断する */
			if (IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				if (CanKeepSlidingState())
				{
					return FindState(PenguinSlidingState::ID());
				}
				else
				{
					return FindState(PenguinSlideEndState::ID());
				}
			}

			/** スライドを始められるならスライド開始状態へ */
			if (CanChangeSlideStartState())
			{
				return FindState(PenguinSlideStartState::ID());
			}

			/** ダッシュ入力があり、移動入力があればダッシュ状態へ */
			if (CanChangeRunState())
			{
				return FindState(PenguinRunState::ID());
			}

			/** 移動入力があればスニーク状態へ */
			if (CanChangeMoveState())
			{
				return FindState(PenguinSneakState::ID());
			}

			/** 当てはまらなければ待機状態へ */
			return FindState(PenguinIdleState::ID());
		}
	}
}