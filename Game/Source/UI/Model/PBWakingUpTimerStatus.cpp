/**
 * @file PBWakingUpTimerStatus.cpp
 * @brief PB起床タイマー専用のステータスクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "MasterPBWakingUpTimerParameter.h"
#include "PBWakingUpTimerStatus.h"
#include "Source/Core/ParameterManager.h"


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
			, m_ratioProgress(0.0f)
			, m_degreeValue(0.0f)
			, m_degreeMaxValue(0.0f)
			, m_initialPosZ(0.0f)
			, m_resetValue(0.0f)
			, m_offsetPosY(0.0f)
			, m_arrowPivot(Vector2::Zero)
			, m_skeltonColor(Vector4::White)
		{
			// JSONファイルからパラメーターを読み込む。
			core::ParameterManager::Get()->LoadParameter<MasterPBWakingUpTimerParameter>(JSON_PATH, [](const nlohmann::json& j, MasterPBWakingUpTimerParameter& parameter)
				{
					parameter.timerFirstValue = util::JsonConverter::ToFloat(j, "timerFirstValue");
					parameter.timerSecondValue = util::JsonConverter::ToFloat(j, "timerSecondValue");
					parameter.timerThirdValue = util::JsonConverter::ToFloat(j, "timerThirdValue");
					parameter.timerFourthValue = util::JsonConverter::ToFloat(j, "timerFourthValue");
					parameter.offsetValueY = util::JsonConverter::ToFloat(j, "offsetValueY");
					parameter.ratioProgress = util::JsonConverter::ToFloat(j, "ratioProgress");
					parameter.degreeValue = util::JsonConverter::ToFloat(j, "degreeValue");
					parameter.degreeMaxValue = util::JsonConverter::ToFloat(j, "degreeMaxValue");
					parameter.initialPosZ = util::JsonConverter::ToFloat(j, "initialPosZ");
					parameter.resetValue = util::JsonConverter::ToFloat(j, "resetValue");
					parameter.offsetPosY = util::JsonConverter::ToFloat(j, "offsetPosY");
					parameter.arrowPivot = util::JsonConverter::ToVector2(j, "arrowPivot");
					parameter.skeltonColor = util::JsonConverter::ToVector4(j, "skeltonColor");
				});
		}


		PBWakingUpTimerStatus::~PBWakingUpTimerStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterPBWakingUpTimerParameter>();
		}


		void PBWakingUpTimerStatus::SetUp()
		{
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterPBWakingUpTimerParameter>();
			m_timerFirstValue = parameter->timerFirstValue;
			m_timerSecondValue = parameter->timerSecondValue;
			m_timerThirdValue = parameter->timerThirdValue;
			m_timerFourthValue = parameter->timerFourthValue;
			m_offsetValueY = parameter->offsetValueY;
			m_ratioProgress = parameter->ratioProgress;
			m_degreeValue = parameter->degreeValue;
			m_degreeMaxValue = parameter->degreeMaxValue;
			m_initialPosZ = parameter->initialPosZ;
			m_resetValue = parameter->resetValue;
			m_offsetPosY = parameter->offsetPosY;
			m_arrowPivot = parameter->arrowPivot;
			m_skeltonColor = parameter->skeltonColor;
		}


		void PBWakingUpTimerStatus::Update()
		{
			SetUp();
		}
	}
}