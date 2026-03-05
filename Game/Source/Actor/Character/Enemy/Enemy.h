/**
 * @file Enemy.h
 * @brief エネミークラス
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;


		/**
		 * @brief エネミークラス
		 */
		class Enemy :public CharacterBase
		{
		public:
			Enemy();
			~Enemy() override = default;


		public:
			EnemyStateMachine* GetEnemyStateMachine() { return m_stateMachine.get(); }


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc)override final;


		private:
			/** ステートマシン */
			std::unique_ptr<EnemyStateMachine>m_stateMachine;
		};
	}
}
