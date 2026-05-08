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

				auto* NotInputPenguinA = GetUI<UIIcon>(Hash32("NotInputJumpIcon"));
				if (NotInputPenguinA) NotInputPenguinA->m_isDraw = false;

				auto* InputPenguinA = GetUI<UIIcon>(Hash32("InputJumpIcon"));
				if (InputPenguinA) InputPenguinA->m_isDraw = true;


				auto* NotInputButtonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
				if (NotInputButtonA) NotInputButtonA->m_isDraw = false;

				auto* InputButtonA = GetUI<UIIcon>(Hash32("InputAbuttonIcon"));
				if (InputButtonA) InputButtonA->m_isDraw = true;
			}
			else
			{
				auto* NotInputPenguinA = GetUI<UIIcon>(Hash32("NotInputJumpIcon"));
				if (NotInputPenguinA) NotInputPenguinA->m_isDraw = true;

				auto* InputPenguinA = GetUI<UIIcon>(Hash32("InputJumpIcon"));
				if (InputPenguinA) InputPenguinA->m_isDraw = false;

				auto* NotInputButtonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
				if (NotInputButtonA) NotInputButtonA->m_isDraw = true;

				auto* InputButtonA = GetUI<UIIcon>(Hash32("InputAbuttonIcon"));
				if (InputButtonA) InputButtonA->m_isDraw = false;
			}

			if (IsInputBButton())
			{
				auto* NotInputPenguinB = GetUI<UIIcon>(Hash32("NotInputSneakIcon"));
				if (NotInputPenguinB) NotInputPenguinB->m_isDraw = false;

				auto* InputPenguinB = GetUI<UIIcon>(Hash32("InputSneakIcon"));
				if (InputPenguinB) InputPenguinB->m_isDraw = true;

				auto* NotInputButtonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
				if (NotInputButtonB) NotInputButtonB->m_isDraw = false;

				auto* InputButtonB = GetUI<UIIcon>(Hash32("InputBbuttonIcon"));
				if (InputButtonB) InputButtonB->m_isDraw = true;
			}
			else
			{
				auto* NotInputPenguinB = GetUI<UIIcon>(Hash32("NotInputSneakIcon"));
				if (NotInputPenguinB) NotInputPenguinB->m_isDraw = true;

				auto* InputPenguinB = GetUI<UIIcon>(Hash32("InputSneakIcon"));
				if (InputPenguinB) InputPenguinB->m_isDraw = false;

				auto* NotInputButtonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
				if (NotInputButtonB) NotInputButtonB->m_isDraw = true;

				auto* InputButtonB = GetUI<UIIcon>(Hash32("InputBbuttonIcon"));
				if (InputButtonB) InputButtonB->m_isDraw = false;
			}

			if (IsInputXButton())
			{
				auto* NotInputPenguinX = GetUI<UIIcon>(Hash32("NotInputSlideIcon"));
				if (NotInputPenguinX) NotInputPenguinX->m_isDraw = false;

				auto* InputPenguinX = GetUI<UIIcon>(Hash32("InputSlideIcon"));
				if (InputPenguinX) InputPenguinX->m_isDraw = true;

				auto* NotInputButtonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
				if (NotInputButtonX) NotInputButtonX->m_isDraw = false;

				auto* InputButtonX = GetUI<UIIcon>(Hash32("InputXbuttonIcon"));
				if (InputButtonX) InputButtonX->m_isDraw = true;
			}
			else
			{
				auto* NotInputPenguinX = GetUI<UIIcon>(Hash32("NotInputSlideIcon"));
				if (NotInputPenguinX) NotInputPenguinX->m_isDraw = true;

				auto* InputPenguinX = GetUI<UIIcon>(Hash32("InputSlideIcon"));
				if (InputPenguinX) InputPenguinX->m_isDraw = false;

				auto* NotInputButtonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
				if (NotInputButtonX) NotInputButtonX->m_isDraw = true;

				auto* InputButtonX = GetUI<UIIcon>(Hash32("InputXbuttonIcon"));
				if (InputButtonX) InputButtonX->m_isDraw = false;
			}

			if (IsInputYButton())
			{
				auto* NotInputPenguinY = GetUI<UIIcon>(Hash32("NotInputOrderIcon"));
				if (NotInputPenguinY) NotInputPenguinY->m_isDraw = false;

				auto* InputPenguinY = GetUI<UIIcon>(Hash32("InputOrderIcon"));
				if (InputPenguinY) InputPenguinY->m_isDraw = true;

				auto* NotInputButtonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
				if (NotInputButtonY) NotInputButtonY->m_isDraw = false;

				auto* InputButtonY = GetUI<UIIcon>(Hash32("InputYbuttonIcon"));
				if (InputButtonY) InputButtonY->m_isDraw = true;
			}
			else
			{
				auto* NotInputPenguinY = GetUI<UIIcon>(Hash32("NotInputOrderIcon"));
				if (NotInputPenguinY) NotInputPenguinY->m_isDraw = true;

				auto* InputPenguinY = GetUI<UIIcon>(Hash32("InputOrderIcon"));
				if (InputPenguinY) InputPenguinY->m_isDraw = false;

				auto* buttonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
				if (buttonY) buttonY->m_isDraw = true;

				auto* InputButtonY = GetUI<UIIcon>(Hash32("InputYbuttonIcon"));
				if (InputButtonY) InputButtonY->m_isDraw = false;
			}
		}


		void InGameButtonMenu::InitializeLogic()
		{
			// 押していないときのペンギンのアイコンを非表示にする
			auto* NotInputPenguinA = GetUI<UIIcon>(Hash32("NotInputJumpIcon"));
			if (NotInputPenguinA) NotInputPenguinA->m_isDraw = false;

			auto* NotInputPenguinB = GetUI<UIIcon>(Hash32("NotInputSneakIcon"));
			if (NotInputPenguinB) NotInputPenguinB->m_isDraw = false;

			auto* NotInputPenguinX = GetUI<UIIcon>(Hash32("NotInputSlideIcon"));
			if (NotInputPenguinX) NotInputPenguinX->m_isDraw = false;

			auto* NotInputPenguinY = GetUI<UIIcon>(Hash32("NotInputOrderIcon"));
			if (NotInputPenguinY) NotInputPenguinY->m_isDraw = false;


			// 押してる時のペンギンのアイコンを非表示にする
			auto* InputPenguinA = GetUI<UIIcon>(Hash32("InputJumpIcon"));
			if (InputPenguinA) InputPenguinA->m_isDraw = false;

			auto* InputPenguinB = GetUI<UIIcon>(Hash32("InputSneakIcon"));
			if (InputPenguinB) InputPenguinB->m_isDraw = false;

			auto* InputPenguinX = GetUI<UIIcon>(Hash32("InputSlideIcon"));
			if (InputPenguinX) InputPenguinX->m_isDraw = false;

			auto* InputPenguinY = GetUI<UIIcon>(Hash32("InputOrderIcon"));
			if (InputPenguinY) InputPenguinY->m_isDraw = false;


			// 押していないときのボタンアイコンを非表示にする
			auto* NotInputButtonA = GetUI<UIIcon>(Hash32("NotInputAbuttonIcon"));
			if (NotInputButtonA) NotInputButtonA->m_isDraw = false;

			auto* NotInputButtonB = GetUI<UIIcon>(Hash32("NotInputBbuttonIcon"));
			if (NotInputButtonB) NotInputButtonB->m_isDraw = false;

			auto* NotInputButtonX = GetUI<UIIcon>(Hash32("NotInputXbuttonIcon"));
			if (NotInputButtonX) NotInputButtonX->m_isDraw = false;

			auto* NotInputButtonY = GetUI<UIIcon>(Hash32("NotInputYbuttonIcon"));
			if (NotInputButtonY) NotInputButtonY->m_isDraw = false;


			//　押したときのボタンアイコンを非表示にする
			auto* InputButtonA = GetUI<UIIcon>(Hash32("InputAbuttonIcon"));
			if (InputButtonA) InputButtonA->m_isDraw = false;

			auto* InputButtonB = GetUI<UIIcon>(Hash32("InputBbuttonIcon"));
			if (InputButtonB) InputButtonB->m_isDraw = false;

			auto* InputButtonX = GetUI<UIIcon>(Hash32("InputXbuttonIcon"));
			if (InputButtonX) InputButtonX->m_isDraw = false;

			auto* InputButtonY = GetUI<UIIcon>(Hash32("InputYbuttonIcon"));
			if (InputButtonY) InputButtonY->m_isDraw = false;
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