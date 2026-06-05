/**
 * @file NaughtyChildPenguinIState.h
 * @brief ヤンチャペンギン固有のステートインターフェース
 * @author 立山
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		class NaughtyChildPenguinStateMachine;

		class NaughtyChildPenguinIState : public core::IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}

			NaughtyChildPenguinIState(NaughtyChildPenguinStateMachine* owner);
			~NaughtyChildPenguinIState() override = default;

		protected:
			NaughtyChildPenguinStateMachine* m_owner;
		};


		/****************************************/

		/**
		 * @brief シロクマへ向かうステート
		 * @details AIがBuildInputToTarget()でシロクマへ誘導する。到達したらWakeBearStateへ
		 */
		class NaughtySeekBearState : public NaughtyChildPenguinIState
		{
			appState(NaughtySeekBearState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;

			NaughtySeekBearState(NaughtyChildPenguinStateMachine* owner);
			~NaughtySeekBearState() override = default;
		};


		/****************************************/

		/**
		 * @brief シロクマを起こすステート
		 * @details 到達後にアニメ（Poke等）を再生。アニメ完了でIdleへ戻る
		 */
		class NaughtyWakeBearState : public NaughtyChildPenguinIState
		{
			appState(NaughtyWakeBearState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;

			NaughtyWakeBearState(NaughtyChildPenguinStateMachine* owner);
			~NaughtyWakeBearState() override = default;
		};

	}
}