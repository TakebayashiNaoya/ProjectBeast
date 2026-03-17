/**
 * @file ChildPenguinStateMachine.h
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStatus;


		/**
		 * @brief 親ペンギンのステートマシンクラス
		 */
		class ChildPenguinStateMachine : public core::StateMachineBase
		{
			// ここに親ペンギン固有のセッター関数を追加していく
		public:
			void PlayAnimation(const uint8_t animationID);


			// ここに親ペンギン固有のゲッター関数を追加していく
		public:
			/** ステートの変更先を取得する */
			core::IState* GetChangeState();


		public:
			ChildPenguinStateMachine(ChildPenguin* player);
			~ChildPenguinStateMachine() = default;


		private:
			/** 親ペンギンのポインタ */
			ChildPenguin* m_player;
		};
	}
}

