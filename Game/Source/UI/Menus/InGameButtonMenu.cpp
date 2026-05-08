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
		{

		}


		void InGameButtonMenu::Update()
		{
			ButtonIconUpdate();

			MenuBase::Update();
		}


		void InGameButtonMenu::ButtonIconUpdate()
		{
			if (IsInputAButton())
			{
				auto* buttonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
				if (buttonA) buttonA->m_isDraw = false;

			}
			else
			{
				auto* buttonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
				if (buttonA) buttonA->m_isDraw = true;
			}

			if (IsInputBButton())
			{
				auto* buttonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
				if (buttonB) buttonB->m_isDraw = false;
			}
			else
			{
				auto* buttonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
				if (buttonB) buttonB->m_isDraw = true;
			}

			if (IsInputXButton())
			{
				auto* buttonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
				if (buttonX) buttonX->m_isDraw = false;

			}
			else
			{
				auto* buttonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
				if (buttonX) buttonX->m_isDraw = true;
			}

			if (IsInputYButton())
			{
				auto* buttonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
				if (buttonY) buttonY->m_isDraw = false;

			}
			else
			{
				auto* buttonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
				if (buttonY) buttonY->m_isDraw = true;
			}
		}


		void InGameButtonMenu::InitializeLogic()
		{
			auto* buttonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
			if (buttonA) buttonA->m_isDraw = false;

			auto* buttonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
			if (buttonB) buttonB->m_isDraw = false;

			auto* buttonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
			if (buttonX) buttonX->m_isDraw = false;

			auto* buttonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
			if (buttonY) buttonY->m_isDraw = false;
		}


		bool InGameButtonMenu::IsInputAButton() const
		{
			if (g_pad[0]->IsPress(enButtonA))
			{
				return true;
			}
			return false;
		}

		bool InGameButtonMenu::IsInputBButton() const
		{
			if (g_pad[0]->IsPress(enButtonB))
			{
				return true;
			}
			return false;
		}

		bool InGameButtonMenu::IsInputXButton() const
		{
			if (g_pad[0]->IsPress(enButtonX))
			{
				return true;
			}
			return false;
		}

		bool InGameButtonMenu::IsInputYButton() const
		{
			if (g_pad[0]->IsPress(enButtonY))
			{
				return true;
			}
			return false;
		}
	}
}