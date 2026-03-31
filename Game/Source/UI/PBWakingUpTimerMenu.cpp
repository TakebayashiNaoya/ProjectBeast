/**
 * @file PBWakingUpTimerMenu.cpp
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PBWakingUpTimerMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// タイマーのオフセット位置（エネミーの頭上）
			constexpr float OFFSET_Y = 250.0f;
		}


		PBWakingUpTimerMenu::PBWakingUpTimerMenu()
			: m_currentPBTime(0.0f)
			, m_targetPosition(Vector3::Zero)
			, m_isDraw(false)
		{}


		void PBWakingUpTimerMenu::Update()
		{
			auto* digit = GetUI<UIDigit>(Hash32("PBWakingUpTimerDigit"));
			if (!digit) return;

			// 描画フラグがfalseなら非表示にして早期リターン
			if (!m_isDraw)
			{
				digit->m_isDraw = false;
				PBWakingUpTimerClass::Update();
				return;
			}

			// ワールド座標をスクリーン座標に変換してDigitの位置をエネミーの頭上に設定
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);
			digit->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y, 0.0f);

			// タイマー値をint型にキャストして表示
			digit->SetNumber(static_cast<int>(m_currentPBTime));
			digit->m_isDraw = true;

			// MenuBaseの更新処理
			PBWakingUpTimerClass::Update();
		}


		void PBWakingUpTimerMenu::InitializeLogic()
		{
			auto* digit = GetUI<UIDigit>(Hash32("PBWakingUpTimerDigit"));
			if (digit) digit->m_isDraw = false;
		}
	}
}