/**
 * @file DaddyPenguinController.h
 * @brief 親ペンギンのプレイヤーコントローラー
 * @author 竹林
 */
#pragma once

namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class DaddyPenguin;
		class DaddyPenguinStateMachine;

		/**
		 * @brief 親ペンギンのコントローラークラス
		 */
		class DaddyPenguinController
		{
		public:
			/**
			 * @brief 更新処理
			 */
			void Update();

		public:
			DaddyPenguinController(DaddyPenguin* owner);
			~DaddyPenguinController() = default;

		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_owner;
			/** ステートマシンへの参照 */
			DaddyPenguinStateMachine* m_stateMachine;
		};
	}
}