/**
 * @file DaddyPenguinIState.h
 * @brief 親ペンギンのステートインターフェース
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class DaddyPenguinStateMachine;
		class DaddyPenguinStatus;


		class DaddyPenguinIState : public core::IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}


		public:
			DaddyPenguinIState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinIState() override = default;


		protected:
			/** ステートのオーナー */
			DaddyPenguinStateMachine* m_owner;
		};


		/**
		 * @brief 親ペンギンの待機ステートクラス
		 */
		class DaddyPenguinIdleState : public DaddyPenguinIState
		{
			appState(DaddyPenguinIdleState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinIdleState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinIdleState() override = default;
		};




		/************************************/


		/**
		 * @brief 親ペンギンのスニークステートクラス
		 */
		class DaddyPenguinSneakState : public DaddyPenguinIState
		{
			appState(DaddyPenguinSneakState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinSneakState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinSneakState() override = default;
		};




		/************************************/


		/**
		 * @brief 親ペンギンのダッシュステートクラス
		 */
		class DaddyPenguinRunState : public DaddyPenguinIState
		{
			appState(DaddyPenguinRunState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinRunState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinRunState() override = default;
		};




		/***************************************/


		/**
		 * @brief 親ペンギンのジャンプステートクラス
		 */
		class DaddyPenguinJumpState : public DaddyPenguinIState
		{
			appState(DaddyPenguinJumpState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinJumpState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinJumpState() override = default;
		};




		/*****************************************/


		/**
		 * @brief 親ペンギンのスライド開始ステートクラス
		 */
		class DaddyPenguinSlideStartState : public DaddyPenguinIState
		{
			appState(DaddyPenguinSlideStartState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinSlideStartState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinSlideStartState() override = default;
		};




		/*****************************************/


		/**
		 * @brief 親ペンギンのスライドステートクラス
		 */
		class DaddyPenguinSlidingState : public DaddyPenguinIState
		{
			appState(DaddyPenguinSlidingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinSlidingState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinSlidingState() override = default;
		};




		/*****************************************/


		/**
		 * @brief 親ペンギンのスライド終了ステートクラス
		 */
		class DaddyPenguinSlideEndState : public DaddyPenguinIState
		{
			appState(DaddyPenguinSlideEndState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinSlideEndState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinSlideEndState() override = default;
		};




		/*****************************************/


		/**
		 * @brief 親ペンギンの命令ステートクラス
		 */
		class DaddyPenguinCommandShoutState : public DaddyPenguinIState
		{
			appState(DaddyPenguinCommandShoutState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinCommandShoutState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinCommandShoutState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンの飛び込みステートクラス
		 */
		class DaddyPenguinDivingState : public DaddyPenguinIState
		{
			appState(DaddyPenguinDivingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinDivingState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinDivingState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンの泳ぎステートクラス
		 */
		class DaddyPenguinSwimmingState : public DaddyPenguinIState
		{
			appState(DaddyPenguinSwimmingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinSwimmingState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinSwimmingState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンの登り開始ステートクラス
		 */
		class DaddyPenguinClimbStartState : public DaddyPenguinIState
		{
			appState(DaddyPenguinClimbStartState);
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinClimbStartState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinClimbStartState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンの登りステートクラス
		 */
		class DaddyPenguinClimbingState : public DaddyPenguinIState
		{
			appState(DaddyPenguinClimbingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinClimbingState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinClimbingState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンの登り終了ステートクラス
		 */
		class DaddyPenguinClimbEndState : public DaddyPenguinIState
		{
			appState(DaddyPenguinClimbEndState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinClimbEndState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinClimbEndState() override = default;
		};




		/***************************************/


		/**
		 * @brief 親ペンギンの被弾ステートクラス
		 */
		class DaddyPenguinDamagedState : public DaddyPenguinIState
		{
			appState(DaddyPenguinDamagedState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinDamagedState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinDamagedState() override = default;
		};




		/****************************************/


		/**
		 * @brief 親ペンギンのダイイングステートクラス
		 */
		class DaddyPenguinDiyingState : public DaddyPenguinIState
		{
			appState(DaddyPenguinDyingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinDiyingState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinDiyingState() override = default;
		};




		/****************************************/


		class DaddyPenguinDeadState : public DaddyPenguinIState
		{
			appState(DaddyPenguinDeadState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinDeadState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinDeadState() override = default;
		};




		/****************************************/


		class DaddyPenguinWinState : public DaddyPenguinIState
		{
			appState(DaddyPenguinWinState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinWinState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinWinState() override = default;
		};




		/****************************************/



		class DaddyPenguinLoseState : public DaddyPenguinIState
		{
			appState(DaddyPenguinLoseState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinLoseState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinLoseState() override = default;


		private:
			/** 演出用タイマー */
			float m_timer = 0.0f;
		};
	}
}