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
	}
}

