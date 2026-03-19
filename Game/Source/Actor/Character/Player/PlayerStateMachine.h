/**
 * @file PlayerStateMachine.h
 * @brief プレイヤーのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class Player;
		class PlayerStatus;


		/**
		 * @brief プレイヤーのステートマシンクラス
		 */
		class PlayerStateMachine : public CharacterStateMachine
		{
		public:



		public:
			/** ステートの変更先を取得する */
			core::IState* GetChangeState();


		public:
			PlayerStateMachine(Player* player);
			~PlayerStateMachine() override = default;


		private:
			/** プレイヤーのポインタ */
			Player* m_player;
		};
	}
}

