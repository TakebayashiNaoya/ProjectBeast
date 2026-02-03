/**
 * @file PlayerIState.h
 * @brief プレイヤーのステートインターフェース
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/ActorStateMachine.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class PlayerStateMachine;


		/**
		 * @brief プレイヤーの待機ステートクラス
		 */
		class PlayerIdleState : public IState
		{
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			PlayerIdleState(ActorStateMachine* stateMachine);
			~PlayerIdleState() override = default;


		private:
			/** ステートマシーン */
			PlayerStateMachine* m_owner;
		};




		/************************************/


		/**
		 * @brief プレイヤーの移動ステートクラス
		 */
		class PlayerMoveState : public IState
		{
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		private:
			PlayerMoveState(ActorStateMachine* stateMachine);
			~PlayerMoveState() override = default;


		private:
			/** ステートマシーン */
			PlayerStateMachine* m_owner;
		};
	}
}

