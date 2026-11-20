/**
 * @file OrcaIState.h
 * @brief シャチのステート
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class OrcaStateMachine;


		/**
		 * @brief シャチのステートの基底
		 */
		class OrcaIState : public core::IState
		{
		public:
			void Enter() override {}
			void Update() override {}
			void Exit() override {}


		public:
			OrcaIState(OrcaStateMachine* owner) : m_owner(owner) {}
			~OrcaIState() override = default;


		protected:
			OrcaStateMachine* m_owner;
		};




		/************************************/


		/**
		 * @brief 泳ぐステート（待機・巡回・探索・咆哮・追跡）
		 * @details 速さと深さは OrcaController が設定する。止まるときは移動方向をゼロにする
		 */
		class OrcaMoveState : public OrcaIState
		{
			appState(OrcaMoveState);
		public:
			void Enter() override final;
			void Update() override final;


		public:
			OrcaMoveState(OrcaStateMachine* owner) : OrcaIState(owner) {}
			~OrcaMoveState() override = default;
		};




		/************************************/


		/**
		 * @brief 噛みつくステート（EnemyAttackState に当たる）
		 * @details mouth.tka を再生しながら泳ぐ。浮上中は口元に水しぶきを出し続ける。
		 *          attackImpactTime でエフェクトとSEを出し、IsAttackImpact を立てる。
		 *          そこから attackHoldTime 経ったら IsAttackFinished を立てる
		 */
		class OrcaAttackState : public OrcaIState
		{
			appState(OrcaAttackState);
		public:
			void Enter() override final;
			void Update() override final;


		public:
			OrcaAttackState(OrcaStateMachine* owner) : OrcaIState(owner) {}
			~OrcaAttackState() override = default;


		private:
			float m_attackTimer = 0.0f;
			bool m_hasFiredEffect = false;
			/** 次に口元の水しぶきを出すまでの時間 */
			float m_splashTimer = 0.0f;
		};




		/************************************/


		/**
		 * @brief 気絶ステート（EnemyStunState に当たる）
		 * @details 専用アニメが無いので、止まって泳ぎのアニメをゆっくり再生する。stunDuration で解ける
		 */
		class OrcaStunState : public OrcaIState
		{
			appState(OrcaStunState);
		public:
			void Enter() override final;
			void Update() override final;
			void Exit() override final;


		public:
			OrcaStunState(OrcaStateMachine* owner) : OrcaIState(owner) {}
			~OrcaStunState() override = default;


		private:
			float m_stunTimer = 0.0f;
		};
	}
}
