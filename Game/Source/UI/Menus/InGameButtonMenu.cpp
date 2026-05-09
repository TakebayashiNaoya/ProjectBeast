/**
 * @file InGameButtonMenu.cpp
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "InGameButtonMenu.h"
#include "Source/Util/CRC32.h"

namespace app
{
	namespace ui
	{
		InGameButtonMenu::InGameButtonMenu()
		{}

		void InGameButtonMenu::Update()
		{
			ButtonIconUpdate();
			MenuBase::Update();
		}

		void InGameButtonMenu::ButtonIconUpdate()
		{
			// UI表示を切り替えるラムダ式（ローカル関数）
			auto updateUI = [&](bool isInput, const char* notInputAct, const char* inputAct, const char* notInputBtn, const char* inputBtn) {
				if (auto* ui = GetUI<UIIcon>(Hash32(notInputAct))) ui->m_isDraw = !isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(inputAct)))    ui->m_isDraw = isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(notInputBtn))) ui->m_isDraw = !isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(inputBtn)))    ui->m_isDraw = isInput;
				};

			// 各ボタンに対して一括処理
			updateUI(IsInputAButton(), "NotInputJumpIcon", "InputJumpIcon", "NotInputAbuttonIcon", "InputAbuttonIcon");
			updateUI(IsInputBButton(), "NotInputSneakIcon", "InputSneakIcon", "NotInputBbuttonIcon", "InputBbuttonIcon");
			updateUI(IsInputXButton(), "NotInputSlideIcon", "InputSlideIcon", "NotInputXbuttonIcon", "InputXbuttonIcon");
			updateUI(IsInputYButton(), "NotInputOrderIcon", "InputOrderIcon", "NotInputYbuttonIcon", "InputYbuttonIcon");
		}

		void InGameButtonMenu::InitializeLogic()
		{
			// 初期化対象のアイコン名リスト
			const char* iconNames[] = {
				"NotInputJumpIcon", "NotInputSneakIcon", "NotInputSlideIcon", "NotInputOrderIcon",
				"InputJumpIcon",    "InputSneakIcon",    "InputSlideIcon",    "InputOrderIcon",
				"NotInputAbuttonIcon", "NotInputBbuttonIcon", "NotInputXbuttonIcon", "NotInputYbuttonIcon",
				"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
			};

			// ループですべての該当アイコンを非表示(false)にする
			for (const char* name : iconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_isDraw = false;
				}
			}
		}

		bool InGameButtonMenu::IsInputAButton() const
		{
			return g_pad[0]->IsPress(enButtonA);
		}

		bool InGameButtonMenu::IsInputBButton() const
		{
			return g_pad[0]->IsPress(enButtonB);
		}

		bool InGameButtonMenu::IsInputXButton() const
		{
			return g_pad[0]->IsPress(enButtonX);
		}

		bool InGameButtonMenu::IsInputYButton() const
		{
			return g_pad[0]->IsPress(enButtonY);
		}
	}
}