/**
 * @file EnemyIState.h
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#pragma once
#include "Source/Core/StateMachineBase.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class EnemyStateMachine;


		class EnemyIState :public core::IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}


		public:
			EnemyIState(EnemyStateMachine* owner);
			~EnemyIState() override = default;


		protected:
			EnemyStateMachine* m_owner;
		};




		/************************************/


		/**
		 * @brief エネミーの待機ステートクラス
		 */
		class EnemyIdleState :public EnemyIState
		{
			appState(EnemyIdleState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyIdleState(EnemyStateMachine* owner);
			~EnemyIdleState() override = default;
		};




		/************************************/


		/**
		 * @brief エネミーのスタンステートクラス
		 */
		class EnemyStunState :public EnemyIState
		{
			appState(EnemyStunState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyStunState(EnemyStateMachine* owner);
			~EnemyStunState() override = default;

		private:
			float m_stunTimer;
		};




		/************************************/


		/**
		 * @brief エネミーのサーチ(体を回転)ステートクラス
		 */
		class EnemySearchState :public EnemyIState
		{
			appState(EnemySearchState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemySearchState(EnemyStateMachine* owner);
			~EnemySearchState() override = default;

		private:
			app::SEHandle m_stepSE = -1;
		};




		/************************************/


		/**
		 * @brief エネミーの徘徊ステートクラス
		 */
		class EnemyWalkState :public EnemyIState
		{
			appState(EnemyWalkState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyWalkState(EnemyStateMachine* owner);
			~EnemyWalkState() override = default;


		private:
			app::SEHandle m_stepSE = -1;
			float m_stepTimer = 0.0f;
		};




		/************************************/


		/**
		 * @brief エネミーの追跡ステートクラス
		 */
		class EnemyChaseState :public EnemyIState
		{
			appState(EnemyChaseState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyChaseState(EnemyStateMachine* owner);
			~EnemyChaseState() override = default;


		private:
			app::SEHandle m_stepSE = -1;
		};




		/************************************/


		/**
		 * @brief エネミーのジャンプステートクラス
		 */
		class EnemyJumpState :public EnemyIState
		{
			appState(EnemyJumpState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyJumpState(EnemyStateMachine* owner);
			~EnemyJumpState() override = default;
		};




		/************************************/


		/**
		 * @brief エネミーの泳ぎステートクラス
		 */
		class EnemySwimState :public EnemyIState
		{
			appState(EnemySwimState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemySwimState(EnemyStateMachine* owner);
			~EnemySwimState() override = default;
		};




		/************************************/


		/**
		 * @brief エネミーの攻撃ステートクラス
		 */
		class EnemyAttackState :public EnemyIState
		{
			appState(EnemyAttackState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyAttackState(EnemyStateMachine* owner);
			~EnemyAttackState() override = default;


		private:
			float m_attackTimer = 0.0f;
			bool m_hasFiredEffect = false;
		};




		/************************************/


		/**
		 * @brief エネミーの帰巣ステートクラス
		 */
		class EnemyReturnHomeState :public EnemyIState
		{
			appState(EnemyReturnHomeState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyReturnHomeState(EnemyStateMachine* owner);
			~EnemyReturnHomeState() override = default;


		private:
			app::SEHandle m_stepSE = -1;
		};




		/************************************/


		/**
		 * @brief エネミーのクールダウンステートクラス
		 */
		class EnemyCoolDownState :public EnemyIState
		{
			appState(EnemyCoolDownState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyCoolDownState(EnemyStateMachine* owner);
			~EnemyCoolDownState() override = default;

		private:
			/** 起床ゲージの最大値（満タン=完全に眠っている） */
			static constexpr float MAX_WAKE_UP_GAUGE = 100.0f;
			/** 静寂時のゲージ回復速度（毎秒） */
			static constexpr float GAUGE_RECOVERY_SPEED = 10.0f;
			/** 睡眠タイマーの最大値（秒） */
			static constexpr float MAX_SLEEP_TIME = 30.0f;
		};




		/************************************/


		/**
		 * @brief エネミーの咆哮ステートクラス
		 */
		class EnemyRoarState :public EnemyIState
		{
			appState(EnemyRoarState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyRoarState(EnemyStateMachine* owner);
			~EnemyRoarState() override = default;
		};
	}
}