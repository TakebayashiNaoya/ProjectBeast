/**
 * @file RemainingChildMenu.cpp
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "RemainingChildMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		RemainingChildMenu::RemainingChildMenu()
		{}


		void RemainingChildMenu::Update()
		{
			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = true;

			// 残り子ペンギンの数更新
			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) {
				digit->m_isDraw = true;
				digit->SetNumber(m_childNum);
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void RemainingChildMenu::InitializeLogic()
		{
			// 生成直後は全て非表示にする（UIBaseのデフォルトがm_isDraw=trueのため）
			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = false;

			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) digit->m_isDraw = false;
		}
	}
}

