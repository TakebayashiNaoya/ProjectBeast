/**
 * @file IglooPromptMenu.cpp
 * @brief かまくら入口でAボタンアイコンを表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "IglooPromptMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 親ペンギンのワールド座標からどれだけ上にオフセットするか */
			constexpr float OFFSET_Y = 250.0f;
		}


		IglooPromptMenu::IglooPromptMenu()
			: m_targetPosition(Vector3::Zero)
			, m_isDraw(false)
			, m_promptType(PromptType::None)
		{}


		void IglooPromptMenu::Update()
		{
			// ※ UIエディタで設定したAボタンアイコンのハッシュ名に合わせてください
			//auto* icon = GetUI<UIIcon>(Hash32("IglooPromptIconA"));
			//if (!icon)
			//{
			//	MenuBase::Update();
			//	return;
			//}

			auto* iconA = GetUI<UIIcon>(Hash32("IglooPromptIconA"));
			auto* textEnter = GetUI<UIIcon>(Hash32("IglooEnterText"));
			auto* textExit = GetUI<UIIcon>(Hash32("IglooExitText"));
			// 非表示のとき（None）は全て隠して終わる
			if (m_promptType == PromptType::None)
			{
				if (iconA) iconA->m_isDraw = false;
				if (textEnter) textEnter->m_isDraw = false;
				if (textExit) textExit->m_isDraw = false;
				MenuBase::Update();
				return;
			}

			// 親ペンギンのワールド座標をスクリーン座標に変換
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);

			if (iconA) {
				iconA->m_isDraw = true;
				iconA->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y + 50.0f, 0.0f);
			}
			if (m_promptType == PromptType::Enter)
			{
				if (textEnter) {
					textEnter->m_isDraw = true;
					textEnter->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y - 20.0f, 0.0f);
				}
				if (textExit) { textExit->m_isDraw = false; }
			}
			else if (m_promptType == PromptType::Exit)
			{
				if (textEnter) { textEnter->m_isDraw = false; }
				if (textExit) {
					textExit->m_isDraw = true;
					textExit->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y - 20.0f, 0.0f);
				}
			}

			MenuBase::Update();
		}


		void IglooPromptMenu::InitializeLogic()
		{
			// 生成直後は非表示にする（UIエディタ側でm_isDraw=trueがデフォルトのため）
			auto* icon = GetUI<UIIcon>(Hash32("IglooPromptIconA"));
			if (icon) icon->m_isDraw = false;

			auto* enterText = GetUI<UIIcon>(Hash32("IglooEnterText"));
			if (enterText) enterText->m_isDraw = false;

			auto* exitText = GetUI<UIIcon>(Hash32("IglooExitText"));
			if (exitText) exitText->m_isDraw = false;
		}
	}
}