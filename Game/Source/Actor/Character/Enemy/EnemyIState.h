/**
 * @file EnemyIState.h
 * @brief エネミーのステートインターフェース
 * @author 立山
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


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
		 * @brief エネミーの待機ステートクラス
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
		};
	}
}

