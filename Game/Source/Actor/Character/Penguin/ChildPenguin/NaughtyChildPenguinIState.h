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




		/****************************************/


		/**
		 * @brief 渦潮に飛び込むステート
		 * @details 渦潮に到達後、LaunchBeginアニメを再生する。巻き込まれるまで入力はゼロに保つ
		 */
		class NaughtyDiveWhirlpoolState : public NaughtyChildPenguinIState
		{
			appState(NaughtyDiveWhirlpoolState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;

			NaughtyDiveWhirlpoolState(NaughtyChildPenguinStateMachine* owner);
			~NaughtyDiveWhirlpoolState() override = default;
		};
	}
}