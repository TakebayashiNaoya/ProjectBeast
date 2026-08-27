/**
 * @file TutorialMenu.cpp
 * @brief チュートリアル（ルール説明）画面の動的処理クラス
 */
#include "stdafx.h"
#include "TutorialMenu.h"
#include "UIMenuConstants.h"

#include "Source/Sound/SoundManager.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** ページ数。画像を追加・削除した場合はこの値を変更する */
			constexpr int TUTORIAL_PAGE_COUNT = 6;

			/** 各ページ画像のUIIconキー */
			constexpr uint32_t TUTORIAL_PAGE_ICON_KEYS[TUTORIAL_PAGE_COUNT] =
			{
					Hash32("TutorialPage1Icon")
				,	Hash32("TutorialPage2Icon")
				,	Hash32("TutorialPage3Icon")
				,	Hash32("TutorialPage4Icon")
				,	Hash32("TutorialPage5Icon")
				,	Hash32("TutorialPage6Icon")
			};

			/** 現在ページ数字のUIDigitキー */
			constexpr uint32_t TUTORIAL_CURRENT_PAGE_DIGIT_KEY = Hash32("TutorialCurrentPageDigit");

			/** 総ページ数字のUIDigitキー */
			constexpr uint32_t TUTORIAL_TOTAL_PAGE_DIGIT_KEY = Hash32("TutorialTotalPageDigit");

			/** スラッシュアイコンのUIIconキー */
			constexpr uint32_t TUTORIAL_SLASH_ICON_KEY = Hash32("TutorialSlashIcon");
		}


		TutorialMenu::TutorialMenu()
			: m_currentPage(0)
			, m_isClosed(false)
			, m_isStickNeutralX(true)
			, m_axisInputDetector()
			, m_cursorSelector(TUTORIAL_PAGE_COUNT)
		{}


		void TutorialMenu::Update()
		{
			// 閉じるフラグをリセット。
			if (m_isClosed) m_isClosed = false;

			// ページ切り替え入力の処理。
			UpdatePageInput();

			// 表示ページの切り替え。
			UpdatePageVisibility();

			// ページ番号の更新。
			UpdatePageDigit();

			// キャンバスの更新。
			TutorialClass::Update();
		}


		void TutorialMenu::InitializeLogic()
		{
			// ページアイコンの初期化。
			InitializePageIcon();

			// ページ番号UIの初期化。
			InitializePageNumber();

			m_axisInputDetector.Reset();
			m_cursorSelector.Reset();
		}


		void TutorialMenu::InitializePageIcon()
		{
			// 全ページアイコンを非表示にしてから最初のページのみ表示する。
			for (int i = 0; i < TUTORIAL_PAGE_COUNT; i++)
			{
				auto* icon = GetUI<UIIcon>(TUTORIAL_PAGE_ICON_KEYS[i]);
				if (icon == nullptr) continue;
				icon->SetIsDraw(false);
			}

			// 最初のページを表示。
			auto* firstIcon = GetUI<UIIcon>(TUTORIAL_PAGE_ICON_KEYS[0]);
			if (firstIcon)
			{
				firstIcon->SetIsDraw(true);
			}

			// ページインデックスをリセット。
			m_cursorSelector.Reset();
			m_currentPage = m_cursorSelector.Get();
		}


		void TutorialMenu::InitializePageNumber()
		{
			// 総ページ数を設定。
			auto* totalDigit = GetUI<UIDigit>(TUTORIAL_TOTAL_PAGE_DIGIT_KEY);
			if (totalDigit)
			{
				totalDigit->SetNumber(TUTORIAL_PAGE_COUNT);
			}

			// 現在ページ（1始まり）を設定。
			auto* currentDigit = GetUI<UIDigit>(TUTORIAL_CURRENT_PAGE_DIGIT_KEY);
			if (currentDigit)
			{
				currentDigit->SetNumber(m_currentPage + 1);
			}
		}


		void TutorialMenu::UpdatePageInput()
		{
			// Bボタンで閉じる。
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				m_isClosed = true;
				return;
			}

			const float stickX = g_pad[0]->GetLStickXF();

			// 右入力（十字キーまたはスティック右）で次のページへ、左入力で前のページへ。
			// Positive=右（+1）、Negative=左（-1）として、ページ切り替えとSE再生をまとめて行う。
			const auto dir = m_axisInputDetector.Update(
				stickX, g_pad[0]->IsTrigger(enButtonLeft), g_pad[0]->IsTrigger(enButtonRight), STICK_THRESHOLD);

			if (m_cursorSelector.TryMove(dir))
			{
				m_currentPage = m_cursorSelector.Get();
			}
		}


		void TutorialMenu::UpdatePageVisibility()
		{
			for (int i = 0; i < TUTORIAL_PAGE_COUNT; i++)
			{
				auto* icon = GetUI<UIIcon>(TUTORIAL_PAGE_ICON_KEYS[i]);
				if (icon == nullptr) continue;
				icon->SetIsDraw(i == m_currentPage);
			}
		}


		void TutorialMenu::UpdatePageDigit()
		{
			// 現在ページ（1始まり）を反映。
			auto* currentDigit = GetUI<UIDigit>(TUTORIAL_CURRENT_PAGE_DIGIT_KEY);
			if (currentDigit)
			{
				currentDigit->SetNumber(m_currentPage + 1);
			}
		}
	}
}