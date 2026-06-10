/**
 * @file TutorialWindowMenu.h
 * @brief チュートリアルポップアップウィンドウ（ひな型）
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Animation/UIAnimation.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief チュートリアルウィンドウメニュー
		 * @detail マリオ64風のポップアップウィンドウ。
		 *
		 *   ■ アニメーション フロー（UIScaleAnimation を使用）
		 *     Closed → Opening（BG スケール 0→1、EaseOut 0.3秒）
		 *           → Opened（動画・説明表示、B ボタン入力待ち）
		 *           → Closing（コンテンツ非表示、BG スケール 1→0、EaseIn 0.2秒）
		 *           → Closed（IsClosedByUser() = true、1フレームのみ）
		 *
		 *   ■ 使い方（TutorialInGameScene.cpp を参照）
		 *     1. Layout を宣言して Initialize<TutorialWindowMenu>(json) で初期化。
		 *     2. Open() を呼ぶ。
		 *     3. 毎フレーム Layout::Update() / Layout::Render() を呼ぶ。
		 *     4. IsClosedByUser() が true のフレームに次の処理へ進む。
		 *
		 *   ■ JSON に必要な要素（name を変えないこと）
		 *     TutorialWindowBg          : 背景パネル（UIIcon）※ scale を [0,0,1] にすること
		 *     TutorialWindowVideo       : 動画（UIVideo）
		 *     TutorialWindowDesc        : 説明画像（UIIcon）
		 *     TutorialWindowClosePrompt : 操作案内画像（UIIcon）
		 */
		class TutorialWindowMenu : public MenuBase
		{
			using Base = MenuBase;

		public:
			TutorialWindowMenu();

			void Update() override;
			void InitializeLogic() override;

		public:
			/** ウィンドウを開く（Opening アニメーション開始） */
			void Open();

			/** プログラムからウィンドウを即座に閉じる（アニメーションなし） */
			void Close();

			/** ウィンドウが完全に閉じているか */
			bool IsClosed() const { return m_state == State::Closed; }

			/** ウィンドウが開いているか（アニメーション中を含む） */
			bool IsOpen() const { return m_state != State::Closed; }

			/** 今フレームに閉じアニメーションが完了したか（1フレームのみ true） */
			bool IsClosedByUser() const { return m_closedByUser; }

		private:
			enum class State { Closed, Opening, Opened, Closing };

			void UpdateInput();
			void CheckAnimationComplete();
			void SetContentVisible(bool visible);
			void SetAllVisible(bool visible);

		private:
			State m_state = State::Closed;
			bool  m_closedByUser = false;

			/** UIScaleAnimation のキー */
			static constexpr uint32_t ANIM_OPEN = Hash32("TutorialWindowBgOpen");
			static constexpr uint32_t ANIM_CLOSE = Hash32("TutorialWindowBgClose");

			static constexpr float OPEN_DURATION = 0.3f;
			static constexpr float CLOSE_DURATION = 0.2f;
		};
	}
}
