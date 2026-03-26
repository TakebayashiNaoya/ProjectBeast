/**
 * @file PenguinIState.h
 * @brief ペンギン共通のステートインターフェース
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class PenguinStateMachine;
		class PenguinStatus;


		/**
		 * @brief ペンギン共通のステートインターフェース
		 */
		class PenguinIState : public core::IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}


		public:
			PenguinIState(PenguinStateMachine* owner);
			~PenguinIState() override = default;


		protected:
			/** ステートのオーナー */
			PenguinStateMachine* m_owner;
		};


		/**
		 * @brief ペンギンの待機ステートクラス
		 */
		class PenguinIdleState : public PenguinIState
		{
			appState(PenguinIdleState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinIdleState(PenguinStateMachine* owner);
			~PenguinIdleState() override = default;
		};




		/************************************/


		/**
		 * @brief ペンギンのスニークステートクラス
		 */
		class PenguinSneakState : public PenguinIState
		{
			appState(PenguinSneakState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinSneakState(PenguinStateMachine* owner);
			~PenguinSneakState() override = default;
		};




		/************************************/


		/**
		 * @brief ペンギンのダッシュステートクラス
		 */
		class PenguinRunState : public PenguinIState
		{
			appState(PenguinRunState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinRunState(PenguinStateMachine* owner);
			~PenguinRunState() override = default;
		};




		/***************************************/


		/**
		 * @brief ペンギンのジャンプステートクラス
		 */
		class PenguinJumpState : public PenguinIState
		{
			appState(PenguinJumpState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinJumpState(PenguinStateMachine* owner);
			~PenguinJumpState() override = default;
		};




		/*****************************************/


		/**
		 * @brief ペンギンのスライド開始ステートクラス
		 */
		class PenguinSlideStartState : public PenguinIState
		{
			appState(PenguinSlideStartState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinSlideStartState(PenguinStateMachine* owner);
			~PenguinSlideStartState() override = default;
		};




		/*****************************************/


		/**
		 * @brief ペンギンのスライドステートクラス
		 */
		class PenguinSlidingState : public PenguinIState
		{
			appState(PenguinSlidingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinSlidingState(PenguinStateMachine* owner);
			~PenguinSlidingState() override = default;
		};




		/*****************************************/


		/**
		 * @brief ペンギンのスライド終了ステートクラス
		 */
		class PenguinSlideEndState : public PenguinIState
		{
			appState(PenguinSlideEndState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinSlideEndState(PenguinStateMachine* owner);
			~PenguinSlideEndState() override = default;
		};




		/*****************************************/


		/**
		 * @brief ペンギンの命令ステートクラス
		 */
		class PenguinCommandShoutState : public PenguinIState
		{
			appState(PenguinCommandShoutState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinCommandShoutState(PenguinStateMachine* owner);
			~PenguinCommandShoutState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの飛び込みステートクラス
		 */
		class PenguinDivingState : public PenguinIState
		{
			appState(PenguinDivingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinDivingState(PenguinStateMachine* owner);
			~PenguinDivingState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの泳ぎステートクラス
		 */
		class PenguinSwimmingState : public PenguinIState
		{
			appState(PenguinSwimmingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinSwimmingState(PenguinStateMachine* owner);
			~PenguinSwimmingState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの登り開始ステートクラス
		 */
		class PenguinClimbStartState : public PenguinIState
		{
			appState(PenguinClimbStartState);
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinClimbStartState(PenguinStateMachine* owner);
			~PenguinClimbStartState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの登りステートクラス
		 */
		class PenguinClimbingState : public PenguinIState
		{
			appState(PenguinClimbingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinClimbingState(PenguinStateMachine* owner);
			~PenguinClimbingState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの登り終了ステートクラス
		 */
		class PenguinClimbEndState : public PenguinIState
		{
			appState(PenguinClimbEndState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinClimbEndState(PenguinStateMachine* owner);
			~PenguinClimbEndState() override = default;
		};




		/***************************************/


		/**
		 * @brief ペンギンの被弾ステートクラス
		 */
		class PenguinDamagedState : public PenguinIState
		{
			appState(PenguinDamagedState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinDamagedState(PenguinStateMachine* owner);
			~PenguinDamagedState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンのダイイングステートクラス
		 */
		class PenguinDiyingState : public PenguinIState
		{
			appState(PenguinDyingState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinDiyingState(PenguinStateMachine* owner);
			~PenguinDiyingState() override = default;
		};




		/****************************************/


		/**
		 * @brief ペンギンの死亡ステートクラス
		 */
		class PenguinDeadState : public PenguinIState
		{
			appState(PenguinDeadState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PenguinDeadState(PenguinStateMachine* owner);
			~PenguinDeadState() override = default;
		};
	}
}
