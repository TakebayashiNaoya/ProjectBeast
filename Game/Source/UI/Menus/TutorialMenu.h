/**
 * @file TutorialMenu.h
 * @brief チュートリアル（ルール説明）画面の動的処理クラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/Input/UICursorSelector.h"
#include "Source/UI/Modules/Input/UIInputController.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief チュートリアル画面メニュー
		 * @detail ページ画像の切り替えと、ページ番号の表示を担当する。
		 *         現在は UIIcon（1枚画像）で表示しているが、
		 *         将来的に UIパーツ方式へ切り替える場合は
		 *         InitializeLogic() の中身のみを修正すればよい。
		 */
		class TutorialMenu : public MenuBase
		{
			using TutorialClass = MenuBase;

		public:
			TutorialMenu();

			void Update()override;
			void InitializeLogic()override;

		public:
			/**
			 * @brief ページアイコンの初期化
			 */
			void InitializePageIcon();

			/**
			 * @brief ページ番号UIの初期化
			 */
			void InitializePageNumber();

			/**
			 * @brief ページ切り替え入力の処理
			 */
			void UpdatePageInput();

			/**
			 * @brief 現在のページに合わせてアイコン表示を切り替える
			 */
			void UpdatePageVisibility();

			/**
			 * @brief 現在のページ番号をDigitに反映する
			 */
			void UpdatePageDigit();

		public:
			/**
			 * @brief 閉じるフラグを取得
			 * @return 閉じるフラグ
			 */
			bool IsClosed()const { return m_isClosed; }

			/**
			 * @brief 閉じるフラグを設定
			 * @param isClosed 閉じるフラグ
			 */
			void SetClosed(bool isClosed) { m_isClosed = isClosed; }

		private:
			/** 現在のページインデックス（0始まり） */
			int m_currentPage;
			/** 閉じるフラグ */
			bool m_isClosed;
			/** スティック左右ニュートラル判定 */
			bool m_isStickNeutralX;

			/** コントローラの入力制御 */
			AxisInputDetector m_axisInputDetector;
			/** カーソル移動時の制御 */
			CursorIndexSelector m_cursorSelector;
		};
	}
}