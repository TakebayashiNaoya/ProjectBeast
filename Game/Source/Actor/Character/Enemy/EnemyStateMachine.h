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
			 * @brief 座標の取得
			 */
			const Vector3& GetPosition() const { return m_transform.m_position; }


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
			void SetFindPenguin(const bool isFindPenguin) { m_isFindPenguin = isFindPenguin; }
			/**
			 * @brief
			 */
			bool IsFindPenguin()const { return m_isFindPenguin; }


			/**
			 * @brief 索敵しているかの設定
			 */
			void SetSeach(const bool isSeach) { m_isSeach = isSeach; }
			/**
			 * @brief
			 */
			bool IsSeach()const { return m_isSeach; }


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


			/**
			 * @brief　スタンしているかの設定
			 */
			void SetStun(const bool isStun) { m_isStun = isStun; }
			/**
			 * @brief スタンしているかの取得
			 */
			bool IsStun() const { return m_isStun; }


			/**
			 * @brief 帰巣するかの設定
			 */
			void SetReturnHome(const bool returnHome) { m_isReturnHome = returnHome; }
			/**
			 * @brief 帰巣するかの取得
			 */
			bool IsReturnHome()const { return m_isReturnHome; }


			/**
			 * @brief クールダウンするかの設定
			 */
			void SetCoolDown(const bool coolDown) { m_isCoolDown = coolDown; }
			/**
			 * @brief クールダウンしているかの取得
			 */
			bool IsCoolDown()const { return m_isCoolDown; }


			/**
			 * @brief 攻撃中かの設定
			 */
			void SetIsAttack(const bool coolDown) { m_isAttackPlaying = coolDown; }
			/**
			 * @brief 攻撃中かの取得
			 */
			bool IsAttack()const { return m_isAttackPlaying; }


			/**
			 * @brief 咆哮中かの設定
			 */
			void SetIsRoar(const bool roar) { m_isRoar = roar; }
			/**
			 * @brief 咆哮中かの取得
			 */
			bool IsRoar()const { return m_isRoar; }


			/**
			 * @brief 起床ゲージのゲッター
			 */
			float GetWakeUpGauge() const { return m_wakeUpGauge; }
			/**
			 * @brief 起床ゲージのセッター
			 */
			void SetWakeUpGauge(float gauge) { m_wakeUpGauge = gauge; }


			/**
			 * @brief 睡眠タイマーのゲッター
			 */
			float GetSleepTimer() const { return m_sleepTimer; }
			/**
			 * @brief 睡眠タイマーのセッター
			 */
			void SetSleepTimer(float timer) { m_sleepTimer = timer; }


			/**
			 * @brief 索敵目標座標のゲッター
			 */
			const Vector3& GetSearchTargetPos() const { return m_searchTargetPos; }
			/**
			 * @brief 索敵目標座標のセッター
			 */
			void SetSearchTargetPos(const Vector3& pos) { m_searchTargetPos = pos; }

			/**
			 * @brief 追跡（チェイス）状態かどうかを設定
			 */
			void SetIsChasing(const bool isChasing) { m_isChasing = isChasing; }
			/**
			 * @brief 追跡（チェイス）状態かどうかを取得
			 * @return Chase中なら true
			 */
			bool IsChasing() const { return m_isChasing; }


			/**
			 * @brief　攻撃して叩きつけたかどうかを設定
			 */
			void SetAttackImpact(bool isImpact) { m_isAttackImpact = isImpact; }
			/**
			 * @brief 攻撃して叩きつけたかどうかを取得
			 */
			bool IsAttackImpact() const { return m_isAttackImpact; }


		public:
			bool IsSwim()const;


		private:
			/** 待機状態に変更できるか */
			bool CanChangeIdle() const;
			/** スタン状態に変更できるか */
			bool CanChangeStun() const;
			/** 見回し状態に変更できるか */
			bool CanChangeSearch() const;
			/** 徘徊状態に変更できるか */
			bool CanChangeWalk() const;
			/** チェイス状態に変更できるか */
			bool CanChangeChace() const;
			/** 攻撃状態に変更できるか */
			bool CanChangeAttack() const;
			/** 帰巣状態に変更できるか */
			bool CanChangeReturnHome()const;
			/** クールダウン状態に変更できるか */
			bool CanChangeCoolDown()const;
			/** 咆哮状態に変更できるか */
			bool CanChangeRoar()const;


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

			/** スタン状態か */
			bool m_isStun;

			/** 帰巣する状態か */
			bool m_isReturnHome;

			/** クールダウン状態か */
			bool m_isCoolDown;

			/** 攻撃中かどうか */
			bool m_isAttackPlaying;

			/** 咆哮できるかどうか */
			bool m_isRoar;

			/** 追跡（チェイス）中かどうか */
			bool m_isChasing = false;

			/** エネミーが攻撃して叩きつけたかどうか */
			bool m_isAttackImpact = false;

			/** 起床ゲージ（満タン=完全に眠っている、0=起きる） */
			float m_wakeUpGauge = 0.0f;

			/** 睡眠タイマー（30秒から減り続け、0になると起きる） */
			float m_sleepTimer = 0.0f;

			/** 索敵時の目標座標 */
			Vector3 m_searchTargetPos = Vector3::Zero;
		};
	}
}