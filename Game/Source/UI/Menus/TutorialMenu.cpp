/**
 * @file TutorialMenu.cpp
 * @brief チュートリアル（ルール説明）画面の動的処理クラス
 * @author 竹林
 */
#include "stdafx.h"
#include "TutorialMenu.h"
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

			constexpr float STICK_THRESHOLD = 0.5f;
		}


		TutorialMenu::TutorialMenu()
			: m_currentPage(0)
			, m_isClosed(false)
			, m_isStickNeutralX(true)
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
			m_currentPage = 0;
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
			if (fabsf(stickX) < STICK_THRESHOLD) m_isStickNeutralX = true;

			// 右入力（十字キーまたはスティック右）で次のページへ。
			if (g_pad[0]->IsTrigger(enButtonRight) || (m_isStickNeutralX && stickX > STICK_THRESHOLD))
			{
				m_currentPage = (m_currentPage + 1) % TUTORIAL_PAGE_COUNT;
				m_isStickNeutralX = false;
			}
			// 左入力（十字キーまたはスティック左）で前のページへ。
			else if (g_pad[0]->IsTrigger(enButtonLeft) || (m_isStickNeutralX && stickX < -STICK_THRESHOLD))
			{
				m_currentPage = (m_currentPage - 1 + TUTORIAL_PAGE_COUNT) % TUTORIAL_PAGE_COUNT;
				m_isStickNeutralX = false;
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