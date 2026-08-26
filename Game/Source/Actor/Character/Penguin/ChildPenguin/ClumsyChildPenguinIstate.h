/**
 * @file ClumsyChildPenguinIState.h
 * @brief おっちょこちょいペンギン固有のステートインターフェース
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ClumsyChildPenguinStateMachine;

		using SEHandle = uint32_t;


		/**
		 * @brief おっちょこちょいペンギン固有のステートインターフェース
		 * @note すべてのおっちょこちょいペンギン固有ステートはこのクラスを継承する
		 */
		class ClumsyChildPenguinIState : public core::IState
		{
		public:
			virtual void Enter() override {}
			virtual void Update() override {}
			virtual void Exit() override {}


		public:
			ClumsyChildPenguinIState(ClumsyChildPenguinStateMachine* owner);
			~ClumsyChildPenguinIState() override = default;


		protected:
			/** ステートのオーナー */
			ClumsyChildPenguinStateMachine* m_owner;
		};




		/****************************************/


		/**
		 * @brief おっちょこちょいペンギンの転倒ステートクラス
		 * @details 歩き・走り中に確率で遷移する。Tripアニメを再生してStandUpStateへ渡す
		 */
		class ClumsyTripState : public ClumsyChildPenguinIState
		{
			appState(ClumsyTripState);
		public:
			/** IStateの仮想関数のオーバーライド */
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			ClumsyTripState(ClumsyChildPenguinStateMachine* owner);
			~ClumsyTripState() override = default;
		};




		/****************************************/


		/**
		 * @brief おっちょこちょいペンギンの起き上がりステートクラス
		 * @details TripStateまたはSlipStateの後に遷移する。StandUpアニメを再生して終了する
		 */
		class ClumsyStandUpState : public ClumsyChildPenguinIState
		{
			appState(ClumsyStandUpState);
		public:
			/** IStateの仮想関数のオーバーライド */
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			ClumsyStandUpState(ClumsyChildPenguinStateMachine* owner);
			~ClumsyStandUpState() override = default;
		};




		/****************************************/


		/**
		 * @brief おっちょこちょいペンギンのスリップステートクラス
		 * @details スライド解除時に確率で遷移する。TripアニメとStandUpアニメを続けて再生する
		 */
		class ClumsySlipState : public ClumsyChildPenguinIState
		{
			appState(ClumsySlipState);
		public:
			/** IStateの仮想関数のオーバーライド */
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			ClumsySlipState(ClumsyChildPenguinStateMachine* owner);
			~ClumsySlipState() override = default;
		};
	}
}