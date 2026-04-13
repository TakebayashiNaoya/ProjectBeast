/**
 * @file IglooPromptMenu.cpp
 * @brief かまくら入口でAボタンアイコンを表示するクラス
 * @author （担当者名）
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
		{}


		void IglooPromptMenu::Update()
		{
			// ※ UIエディタで設定したAボタンアイコンのハッシュ名に合わせてください
			auto* icon = GetUI<UIIcon>(Hash32("IglooPromptIconA"));
			if (!icon)
			{
				MenuBase::Update();
				return;
			}

			// 描画フラグが false のとき、起動直後に原点で一瞬表示されるバグを防ぐため
			// 位置計算をスキップして非表示にする
			if (!m_isDraw)
			{
				icon->m_isDraw = false;
				MenuBase::Update();
				return;
			}

			// 親ペンギンのワールド座標をスクリーン座標に変換
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);

			// アイコンをペンギンの頭上に配置
			icon->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y, 0.0f);
			icon->m_isDraw = true;

			MenuBase::Update();
		}


		void IglooPromptMenu::InitializeLogic()
		{
			// 生成直後は非表示にする（UIエディタ側でm_isDraw=trueがデフォルトのため）
			auto* icon = GetUI<UIIcon>(Hash32("IglooPromptIconA"));
			if (icon) icon->m_isDraw = false;
		}
	}
}