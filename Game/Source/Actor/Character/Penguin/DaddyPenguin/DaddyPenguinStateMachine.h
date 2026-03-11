/**
 * @file DaddyPenguinStateMachine.h
 * @brief 親ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class DaddyPenguin;
		class DaddyPenguinStatus;


		/**
		 * @brief 親ペンギンのステートマシンクラス
		 */
		class DaddyPenguinStateMachine : public StateMachineBase
		{
			// ここに親ペンギン固有のセッター関数を追加していく
		public:
			void PlayAnimation(const uint8_t animationID);


			// ここに親ペンギン固有のゲッター関数を追加していく
		public:
			/** ステートの変更先を取得する */
			IState* GetChangeState();


		public:
			DaddyPenguinStateMachine(DaddyPenguin* player);
			~DaddyPenguinStateMachine() = default;


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_player;
		};
	}
}

