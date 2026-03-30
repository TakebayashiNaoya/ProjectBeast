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
		{
		}


		void RemainingChildMenu::Update()
		{
			// 残り子ペンギンの数更新
			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) {
				digit->SetNumber(m_childNum);
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void RemainingChildMenu::InitializeLogic()
		{
			// TODO: 減った時のアニメーションなど追加したい
		}
	}
}

