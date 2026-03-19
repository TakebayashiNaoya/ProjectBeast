/**
 * @file EnemyStateMachine.h
 * @brief エネミーのステートマシン
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Enemy;
		class EnemyStatus;

		class Player;


		class EnemyStateMachine :public CharacterStateMachine
		{
		public:
			core::IState* GetChangeState();


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

			const EnemyStatus* GetOwnerStatus();


		public:

			/**
			 * @brief 移動ベクトルのゲッター
			 */
			const Vector3& GetMoveVector()const { return m_moveVector; }
			/**
			 * @brief 移動ベクトルのセッター
			 */
			void SetMoveVector(const Vector3& moveVector) { m_moveVector = moveVector; }


			/**
			 * @brief Aボタンを押せるかの設定
			 */
			void SetActionButtonA(const bool isActionButtonA) { m_actionButtonA = isActionButtonA; }
			/**
			 * @brief Aボタンを押したか取得
			 */
			bool IsActionButtonA() const { return m_actionButtonA; }


			/**
			 * @brief Bボタンを押せるかの設定
			 */
			void SetActionButtonB(const bool isActionButtonB) { m_actionButtonB = isActionButtonB; }
			/**
			 * @brief Bボタンを押したか取得
			 */
			bool IsActionButtonB() const { return m_actionButtonB; }


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
			 * @brief ペンギンを発見
			 */
			void SetSeach(const bool isSeach) { m_isSeach = isSeach; }
			/**
			 * @brief
			 */
			bool IsSeach()const { return m_isSeach; }


			/**
			 * @brief ペンギンを発見
			 */
			void SetFindPenguin(const bool isFindPenguin) { m_isFindPenguin = isFindPenguin; }
			/**
			 * @brief
			 */
			bool IsFindPenguin()const { return m_isFindPenguin; }


			/**
			 * @brief 近くのペンギンの設定
			 */
			void SetIsNearPenguin(const bool isNearPneguin) { m_isNearPenguin = isNearPneguin; }


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
			/** 見回し状態に変更できるか */
			bool CanChangeSearch() const;
			/** 徘徊状態に変更できるか */
			bool CanChangeWalk() const;
			/** チェイス状態に変更できるか */
			bool CanChangeChace() const;
			/** 攻撃状態に変更できるか */
			bool CanChangeAttack() const;


		private:
			/** エネミーのポインタ */
			Enemy* m_owner;
			/** エネミーのステータス */
			EnemyStatus* m_ownerStatus;

			/** 今のステータス */
			core::IState* m_currentState;
			core::IState* m_nextState;

			/** 移動ベクトル */
			Vector3 m_moveVector;

			/** プレイヤーの位置 */
			Vector3 m_playerPosition;

			/** プレイヤークラスのポインタ */
			Player* m_targetPlayer;

			/** 左スティックの入力量 */
			float m_stickLAmount;

			/** Aボタンを押せるかどうか */
			bool m_actionButtonA;

			/** Bボタンを押せるかどうか */
			bool m_actionButtonB;

			/** Xボタンを押せるかどうか */
			bool m_actionButtonX;

			/** サーチ状態かどうか */
			bool m_isSeach;

			/** ペンギンを見つけたか */
			bool m_isFindPenguin;

			/** ペンギンが近くにいるか */
			bool m_isNearPenguin;

			/** 攻撃できるか */
			bool m_canAttack;
		};
	}
}

