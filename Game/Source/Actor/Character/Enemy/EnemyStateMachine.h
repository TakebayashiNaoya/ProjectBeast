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

		class Player;


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

			void PlayAnimation(const int animationIndex);


		public:
			/** 方向のゲッター */
			const Vector3& GetDirection() { return m_direction; }
			/** 方向のセッター */
			void SetDirection(const Vector3& direction) { m_direction = direction; }


			/**
			 * @brief 移動ベクトルのゲッター
			 */
			const Vector3& GetMoveVector()const { return m_moveVector; }
			/**
			 * @brief 移動ベクトルのセッター
			 */
			void SetMoveVector(const Vector3& moveVector) { m_moveVector = moveVector; }


			/**
			 * @brief 座標のゲッター
			 */
			const Vector3& GetPosition() const { return m_position; }
			/**
			 * @brief 座標のセッター
			 */
			void SetPosition(const Vector3& position) { m_position = position; }


			/**
			 * @brief スケールのゲッター
			 */
			const Vector3& GetScale()const { return m_scale; }
			/**
			 * @brief スケールのセッター
			 */
			void SetScale(const Vector3& scale) { m_scale = scale; }


			/**
			 * @brief 回転のゲッター
			 */
			const Quaternion& GetRotation()const { return m_rotation; }
			/**
			 * @brief 回転のセッター
			 */
			void SetRotation(const Quaternion& rotation) { m_rotation = rotation; }


			/**
			 * @brief ダッシュの設定
			 */
			void SetDash(const bool isDash) { m_isDash = isDash; }


			/**
			 * @brief Aボタンを押せるかの設定
			 */
			void SetActionButtonA(const bool isActionButtonA) { m_actionButtonA = isActionButtonA; }
			/**
			 * @brief Aボタンを押したか取得
			 */
			bool IsActionButtonA() const { return m_actionButtonA; }


			/**
			 * @brief Xボタンを押せるかの設定
			 */
			void SetActionButtonX(const bool isActionButtonX) { m_actionButtonX = isActionButtonX; }
			/**
			 * @brief Xボタンを押したか取得
			 */
			bool IsActionButtonX() const { return m_actionButtonA; }


			/**
			 * @brief 入力量を設定
			 */
			void SetStickLAmount(const float stickAmount) { m_stickLAmount = stickAmount; }


			/**
			 * @brief 入力量の取得
			 */
			float GetStickLAmount() const { return m_stickLAmount; }


			/**
			 * @brief 近くのペンギンの設定
			 */
			void SetNearPenguin(const bool isNearPneguin) { m_isNearPenguin = isNearPneguin; }


			/**
			 * @brief 攻撃ができるかの設定
			 */
			void SetCanAttack(const bool isCanAttack) { m_canAttack = isCanAttack; }
			/**
			 * @brief 攻撃ができるかの取得
			 */
			bool CanAttack() const { return m_canAttack; }



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

			/** プレイヤーの位置 */
			Vector3 m_playerPosition = Vector3::Zero;

			/** プレイヤークラスのポインタ */
			Player* m_targetPlayer = nullptr;

			/** 左スティックの入力量 */
			float m_stickLAmount = 0.0f;

			/** ダッシュできるかどうか */
			bool m_isDash = false;

			/** Aボタンを押せるかどうか */
			bool m_actionButtonA = false;

			/** Xボタンを押せるかどうか */
			bool m_actionButtonX = false;

			/** ペンギンが近くにいるか */
			bool m_isNearPenguin = false;

			/** 攻撃できるか */
			bool m_canAttack = false;
		};
	}
}

