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




		/****************************************/


		/**
		 * @brief 親ペンギンのかまくらイベントステート
		 */
		class DaddyPenguinEnterIglooState : public DaddyPenguinIState
		{
			appState(DaddyPenguinEnterIglooState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;

		public:
			DaddyPenguinEnterIglooState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinEnterIglooState() override = default;

		private:
			Vector3 m_entrancePos; // 入り口の座標
			bool m_isArrivedEntrance; // 入り口に到着したか
		};




		/****************************************/


		/**
		 * @brief 親ペンギンのかまくら内待機ステート
		 */
		class DaddyPenguinInsideIglooState : public DaddyPenguinIState
		{
			appState(DaddyPenguinInsideIglooState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;

		public:
			DaddyPenguinInsideIglooState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinInsideIglooState() override = default;
		};
	}
}