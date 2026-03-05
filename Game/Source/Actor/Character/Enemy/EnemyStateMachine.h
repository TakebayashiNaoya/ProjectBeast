/**
 * @file EnemyStateMachine.h
 * @brief エネミーのステートマシン
 * @author 立山
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Enemy;
		class EnemyStatus;
		class EnemyIState;


		class EnemyStateMachine :public StateMachineBase
		{
		public:
			IState* GetChangeState();


		public:
			EnemyStateMachine(Enemy* enemy);
			~EnemyStateMachine() = default;

			void Update();
			void ChangeState();


		public:
			/** 初期設定用の関数 */
			void Setup(Enemy* owner);


		public:
			Enemy* GetOwner() { return m_owner; }

			EnemyStatus* GetOwnerStatus() { return m_ownerStatus; }


		private:
			/** 待機状態に変更できるか */
			bool CanChangeIdle() const;
			/** 徘徊状態に変更できるか */
			bool CanChangeWandering() const;
			/** チェイス状態に変更できるか */
			bool CanChangeChace() const;
			/** 攻撃状態に変更できるか */
			bool CanChangeAttack() const;


		private:
			/** エネミーのポインタ */
			Enemy* m_owner = nullptr;
			/** エネミーのステータス */
			EnemyStatus* m_ownerStatus = nullptr;

			/** 今のステータス */
			IState* m_currentState = nullptr;
			IState* m_nextState = nullptr;

			/** プレイヤーの方向を取る変数(スティック入力の方向) */
			Vector3 m_direction = Vector3::Zero;

			/** 移動ベクトル */
			Vector3 m_moveVector = Vector3::Zero;

			/** 座標 */
			Vector3 m_position = Vector3::Zero;
			/** 拡縮 */
			Vector3 m_scale = Vector3::One * 1.0f;
			/** 回転 */
			Quaternion m_rotation = Quaternion::Identity;

			/** 左スティックの入力量 */
			float m_stickLAmount = 0.0f;

			/** ダッシュできるかどうか */
			bool m_isDash = false;
		};
	}
}

