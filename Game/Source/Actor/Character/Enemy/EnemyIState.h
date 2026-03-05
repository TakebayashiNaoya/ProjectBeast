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


		class EnemyIState :public IState
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
		 * @brief エネミーの徘徊ステートクラス
		 */
		class EnemyWanderingState :public EnemyIState
		{
			appState(EnemyWanderingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyWanderingState(EnemyStateMachine* owner);
			~EnemyWanderingState() override = default;
		};




		/************************************/


		/**
		 * @brief エネミーの追跡ステートクラス
		 */
		class EnemyChaceState :public EnemyIState
		{
			appState(EnemyChaceState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			EnemyChaceState(EnemyStateMachine* owner);
			~EnemyChaceState() override = default;
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

