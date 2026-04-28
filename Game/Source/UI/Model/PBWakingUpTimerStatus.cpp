/**
 * @file PBWakingUpTimerStatus.cpp
 * @brief PB起床タイマー専用のステータスクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PBWakingUpTimerStatus.h"
#include "Source/Core/ParameterManager.h"
#include "MasterPBWakingUpTimerParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// JSONファイルのパス。
			const char* JSON_PATH = "Assets/parameter/timer/PBTimer/PBWakingUpTimerParameter.json";
		}


		PBWakingUpTimerStatus::PBWakingUpTimerStatus()
			: m_timerFirstValue(0.0f)
			, m_timerSecondValue(0.0f)
			, m_timerThirdValue(0.0f)
			, m_timerFourthValue(0.0f)
			, m_offsetValueY(0.0f)
			, m_offsetValueX(0.0f)
			, m_greenColor(Vector4::White)
			, m_yellowColor(Vector4::White)
			, m_redColor(Vector4::White)
		{
			// JSONファイルからパラメーターを読み込む。
			core::ParameterManager::Get()->LoadParameter<MasterPBWakingUpTimerParameter>(JSON_PATH, [](const nlohmann::json& j, MasterPBWakingUpTimerParameter& parameter)
				{
					parameter.timerFirstValue = j["timerFirstValue"].get<float>();
					parameter.timerSecondValue = j["timerSecondValue"].get<float>();
					parameter.timserThirdValue = j["timserThirdValue"].get<float>();
					parameter.timerFourthValue = j["timerFourthValue"].get<float>();
					parameter.offsetValueY = j["offsetValueY"].get<float>();
					parameter.offsetValueX = j["offsetValueX"].get<float>();
				});
		}


		PBWakingUpTimerStatus::~PBWakingUpTimerStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterPBWakingUpTimerParameter>();
		}
		
		
		void PBWakingUpTimerStatus::SetUpUI()
		{
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterPBWakingUpTimerParameter>();
			m_timerFirstValue = parameter->timerFirstValue;
			m_timerSecondValue = parameter->timerSecondValue;
			m_timerThirdValue = parameter->timserThirdValue;
			m_timerFourthValue = parameter->timerFourthValue;
			m_offsetValueY = parameter->offsetValueY;
			m_offsetValueX = parameter->offsetValueX;
			m_greenColor = parameter->greenColor;
			m_yellowColor = parameter->yellowColor;
			m_redColor = parameter->redColor;
		}
		
		
		void PBWakingUpTimerStatus::Update()
		{
			SetUpUI();
		}
	}
}