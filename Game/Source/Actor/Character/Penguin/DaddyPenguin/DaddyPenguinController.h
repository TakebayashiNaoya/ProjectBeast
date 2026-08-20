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


		public:
			/**
			 * @brief 隊列にいる甘えん坊の数を取得する
			 * @return 甘えん坊の数
			 * @note プレイログの記録に使う
			 */
			int GetClingyCount() const { return m_clingyCount; }

			/**
			 * @brief 甘えん坊による減速を適用したスピード倍率を取得する
			 * @return スピード倍率（1.0fで等倍、0.8fで最大減速）
			 * @note プレイログの記録に使う
			 */
			float GetSpeedMultiplier() const { return m_speedMultiplier; }


		private:
			/**
			 * @brief 隊列内の甘えん坊の数から減速率を算出する
			 * @details 移動入力の有無にかかわらず毎フレーム呼ぶ
			 */
			void UpdateClingySlow();


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_owner;
			/** ステートマシンへの参照 */
			DaddyPenguinStateMachine* m_stateMachine;

			int   m_clingyCount     = 0;      /** 隊列にいる甘えん坊の数 */
			float m_speedMultiplier = 1.0f;   /** 甘えん坊による減速を適用したスピード倍率 */


		private:
			/** かまくらプロンプトメニューへの参照 */
			ui::IglooPromptMenu* m_iglooPromptMenu;
		};
	}
}