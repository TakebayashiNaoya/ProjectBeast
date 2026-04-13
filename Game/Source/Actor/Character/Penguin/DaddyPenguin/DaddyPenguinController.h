/**
 * @file DaddyPenguinController.h
 * @brief 親ペンギンのプレイヤーコントローラー
 * @author 竹林
 */
#pragma once

namespace app
{
	namespace ui
	{
		class IglooPromptMenu;
	}

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


		public:
			/**
			 * @brief かまくらプロンプトUIを登録する（GameScene等から呼び出す）
			 * @param menu IglooPromptMenu のポインタ
			 */
			void SetIglooPromptMenu(ui::IglooPromptMenu* menu) { m_iglooPromptMenu = menu; }


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_owner;
			/** ステートマシンへの参照 */
			DaddyPenguinStateMachine* m_stateMachine;


		private:
			/** かまくらプロンプトメニューへの参照 */
			ui::IglooPromptMenu* m_iglooPromptMenu;
		};
	}
}