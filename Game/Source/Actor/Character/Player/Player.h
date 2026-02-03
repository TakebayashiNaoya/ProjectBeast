/**
 * @file Player.h
 * @brief プレイヤークラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class PlayerStateMachine;


		/**
		 * @brief プレイヤークラス
		 */
		class Player : public CharacterBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline PlayerStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			Player();
			virtual ~Player() override = default;


		private:
			bool Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<PlayerStateMachine> m_stateMachine;
		};
	}
}

