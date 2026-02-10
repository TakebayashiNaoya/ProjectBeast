/**
 * @file PlayerIState.h
 * @brief プレイヤーのステートインターフェース
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class PlayerStateMachine;


		class PlayerIState : public IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}


		public:
			PlayerIState(PlayerStateMachine* owner);
			~PlayerIState() override = default;


		protected:
			PlayerStateMachine* m_owner;
		};


		/**
		 * @brief プレイヤーの待機ステートクラス
		 */
		class PlayerIdleState : public PlayerIState
		{
			appState(PlayerIdleState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PlayerIdleState(PlayerStateMachine* owner);
			~PlayerIdleState() override = default;
		};




		/************************************/


		/**
		 * @brief プレイヤーの移動ステートクラス
		 */
		class PlayerMoveState : public PlayerIState
		{
			appState(PlayerMoveState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PlayerMoveState(PlayerStateMachine* owner);
			~PlayerMoveState() override = default;
		};
	}
}

