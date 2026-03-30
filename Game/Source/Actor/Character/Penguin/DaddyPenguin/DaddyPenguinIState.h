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

		using SEHandle = uint32_t;


		/**
		 * @brief 親ペンギンのステートインターフェース
		 * @note  すべての親ペンギンのステートはこのクラスを継承する
		 */
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
			SEHandle m_seHandle;
		};




		/****************************************/


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