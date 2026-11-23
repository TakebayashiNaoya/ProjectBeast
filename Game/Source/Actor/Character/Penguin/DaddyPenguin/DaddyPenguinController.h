/**
 * @file DaddyPenguinController.h
 * @brief 親ペンギンのプレイヤーコントローラー
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
			/**
			 * @brief 自動プレイのボット入力を作る
			 * @details 環境変数 BEAST_AUTOPLAY が立っているときだけ使う。
			 *          隊列に入っていない子ペンギンへ向かって移動し、
			 *          クールダウンが明けるたびにYの再集合を呼ぶ。デバッグ用の疑似プレイヤー。
			 * @param inputX         スティックX入力（カメラ相対）への出力
			 * @param inputY         スティックY入力（カメラ相対）への出力
			 * @param stickLength    スティックの倒し具合への出力
			 * @param outRegroupCall 再集合の呼びかけ入力への出力
			 */
			void UpdateAutoplayBot(
				float& inputX, float& inputY, float& stickLength,
				bool& outRegroupCall);


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_owner;
			/** ステートマシンへの参照 */
			DaddyPenguinStateMachine* m_stateMachine;

			/** 空中にいる時間（秒） */
			float m_autoSlideAirTimer = 0.0f;

			/** 自動プレイボットの現在の目標座標 */
			Vector3 m_botTargetPos = Vector3::Zero;
			/** 自動プレイボットが目標を選び直すまでのタイマー（秒） */
			float m_botRepickTimer = 0.0f;
			/** 自動プレイボットが次に再集合を呼ぶまでのタイマー（秒） */
			float m_botRegroupTimer = 0.0f;
			/** 自動プレイボットのスタック検出：動けていない時間（秒） */
			float m_botStuckTimer = 0.0f;
			/** 自動プレイボットのスタック検出：前回チェック時の座標 */
			Vector3 m_botStuckCheckPos = Vector3::Zero;


		private:
			/** かまくらプロンプトメニューへの参照 */
			ui::IglooPromptMenu* m_iglooPromptMenu;
		};
	}
}