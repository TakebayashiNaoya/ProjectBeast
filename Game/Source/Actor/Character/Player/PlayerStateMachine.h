/**
 * @file PlayerStateMachine.h
 * @brief プレイヤーのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


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
		class PlayerStateMachine : public StateMachineBase
		{
		public:



		public:
			/** ステートの変更先を取得する */
			IState* GetChangeState();


		public:
			PlayerStateMachine(Player* player);
			~PlayerStateMachine() = default;


		private:
			/** プレイヤーのポインタ */
			Player* m_player;
		};
	}
}

