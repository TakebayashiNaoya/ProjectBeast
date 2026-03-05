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


		enum EnAnimationID : uint8_t
		{
			// うつ伏せで死亡する
			enAnimationID_DeathFaceDown = 0,
			// 仰向けで死亡する
			enAnimationID_DeathFaceUp,
			// 食べ物をついばむ
			enAnimationID_EatCrouching,
			// 食べ物を飲み込む
			enAnimationID_EatDrink,
			// 食べるのをやめる
			enAnimationID_EatRising,
			// その場で翼をばたつかせる
			enAnimationID_IdleFlapingWings,
			// その場で体を振るう
			enAnimationID_IdleShake,
			// その場で眠る
			enAnimationID_IdleSleep,
			// その場で周りを見回す
			enAnimationID_IdleLoocAround,
			// その場で何もしない
			enAnimationID_IdleStanding,
			// 走りながらジャンプする
			enAnimationID_JumpRunning,
			// 歩きながらジャンプする
			enAnimationID_JumpWalking,
			// 水中から出ようとする
			enAnimationID_LaunchBegin,
			// 水中から出て着地する
			enAnimationID_LaunchEnd,
			// 水中から出て空中にいるときに翼を速くばたつかせる
			enAnimationID_LaunchFlapingWingsQuickly,
			// 水中から出て空中にいるときに翼をゆっくりばたつかせる				
			enAnimationID_LaunchFkapingWingsSlowly,
			// 走る
			enAnimationID_MoveRun,
			// 泳ぐ
			enAnimationID_MoveSwim,
			// 歩く
			enAnimationID_MoveWalk,
			// 滑り始める
			enAnimationID_SlideStart,
			// 滑っている
			enAnimationID_Sliding,
			// 起き上がる
			enAnimationID_StandUp,
			// こける
			enAnimationID_Trip,
			// 回れ右
			enAnimationID_TurnBack,
			// 左向け左
			enAnimationID_TurnLeft,
			// 右向け右
			enAnimationID_TurnRight,
		};


		class DaddyPenguinIState : public IState
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
		 * @brief 親ペンギンの移動ステートクラス
		 */
		class DaddyPenguinMoveState : public DaddyPenguinIState
		{
			appState(DaddyPenguinMoveState);
		public:
			// IStateの仮想関数のオーバーライド
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			DaddyPenguinMoveState(DaddyPenguinStateMachine* owner);
			~DaddyPenguinMoveState() override = default;
		};
	}
}

