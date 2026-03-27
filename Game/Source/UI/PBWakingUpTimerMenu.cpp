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
			// 最大タイム。
			constexpr float MAX_TIME = 30.0f;
			// 最小タイム。
			constexpr float MIN_TIME = 0.0f;
			// 誤差。
			constexpr float LIMITE_TIME = 31.0f;
			// 描画距離の値。
			constexpr float LENGTH_DRAW_ACTIVE = 80.0f;
		}


		PBWakingUpTimerDigit::PBWakingUpTimerDigit()
			: m_digit(nullptr)
		{}


		PBWakingUpTimerDigit::~PBWakingUpTimerDigit()
		{}
		
		
		void PBWakingUpTimerDigit::Update()
		{}
		
		
		void PBWakingUpTimerDigit::SetUIDigit(UIDigit * digit)
		{
			m_digit = digit;
			K2_ASSERT(m_digit != nullptr, "登録失敗です。");
		}
		
		
		void PBWakingUpTimerDigit::SetValue(float value)
		{
			if (m_digit != nullptr)
			{
				m_digit->SetNumber(value);
			}
		}
		
		



		/*********************************************/


		PBWakingUpTimerMenu::PBWakingUpTimerMenu()
			: m_currentPBTime(0.0f)
			, m_maxTime(LIMITE_TIME)
			, m_minTime(MIN_TIME)
			, m_isLengthDraw(false)
			, m_isPBTimerActive(false)
			, m_eneCon(nullptr)
			, m_enemy(nullptr)
			, m_daddyPenguin(nullptr)
		{
		}
		
		
		PBWakingUpTimerMenu::~PBWakingUpTimerMenu()
		{}
		
		
		void PBWakingUpTimerMenu::Update()
		{
			// DigitのUIを取得。
			auto* digit = GetUI<UIDigit>(Hash32("PBWakingUpTimerDigit"));

			// 非アクティブの時は早期リターン。
			if (!m_isPBTimerActive)return;

			// アクティブの時に
			if (m_isPBTimerActive)
			{	
				// World座標をスクリーン座標に変換して、DigitのUIの位置をシロクマの上に表示。
				Vector2 screenPos = Vector2::Zero;
				Vector3 targetPos = m_enemy->GetTransform().m_position;

				g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, targetPos);
				digit->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + 250.0f, 0.0f);

				float deltaTime = g_gameTime->GetFrameDeltaTime();
				if (m_currentPBTime > m_minTime)
				{
					m_currentPBTime -= deltaTime;
					if (m_currentPBTime <= m_minTime)
					{
						m_currentPBTime = m_minTime;
						digit->m_isDraw = false;
					}
				}
			}
			else
			{
				m_isLengthDraw = false;
			}

			// float型kからint型にキャスト処理を行う。
			float displayTime = static_cast<int>(m_currentPBTime);
			// ディジットのUIにnumberをセット。
			if (digit)
			{
				digit->SetNumber(displayTime);
			}

			// MenuBaseの更新処理。
			PBWakingUpTimerClass::Update();
		}
		

		void PBWakingUpTimerMenu::ResetTimer()
		{
			m_currentPBTime = MAX_TIME;
			m_isPBTimerActive = false;
			auto* digit = GetUI<UIDigit>(Hash32("PBWakingUpTimerDigit"));
			digit->m_isDraw = false;
		}


		void PBWakingUpTimerMenu::SetIsDraw(bool isDraw)
		{
			for (const auto& digit : m_wakingUpTimeMap)
			{
				digit.second->SetIsDraw(isDraw);
			}
		}
		

		void PBWakingUpTimerMenu::InitializeLogic()
		{}
	}
}