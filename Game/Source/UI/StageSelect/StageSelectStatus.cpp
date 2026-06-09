/**
 * @file StageSelectStatus.cpp
 * @brief StageSelectのステータス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSelectStatus.h"

#include "MasterStageSelectParameter.h"

#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace ui
	{
		StageSelectStatus::StageSelectStatus()
			: m_choicesPositions()
			, m_textColor(Vector4::White)
		{
			core::ParameterManager::Get()->LoadParameter<MasterStageSelectParameter>(
				"Assets/parameter/UI/stageSelect/StageSelectParameter.json",
				[](const nlohmann::json& j, MasterStageSelectParameter& p)
				{
					p.inputInterval = app::util::JsonConverter::ToFloat(j, "inputInterval");
					p.inputThreshold = app::util::JsonConverter::ToFloat(j, "inputThreshold");
					p.easyPosition = app::util::JsonConverter::ToVector3(j, "easyPosition");
					p.normalPosition = app::util::JsonConverter::ToVector3(j, "normalPosition");
					p.hardPosition = app::util::JsonConverter::ToVector3(j, "hardPosition");
					p.backPosition = app::util::JsonConverter::ToVector3(j, "backPosition");
					p.textColor = app::util::JsonConverter::ToVector4(j, "textColor");
				}
			);
		}


		StageSelectStatus::~StageSelectStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterStageSelectParameter>();
		}


		void StageSelectStatus::SetUp()
		{
			auto* param = core::ParameterManager::Get()->GetParameter<MasterStageSelectParameter>();
			if (!param) return;

			auto Choices = [&](EnStageChoices choice)
				{
					return static_cast<uint8_t>(choice);
				};

			m_inputInterval = param->inputInterval;
			m_inputThreshold = param->inputThreshold;
			m_choicesPositions.at(Choices(EnStageChoices::Back)) = param->backPosition;
			m_choicesPositions.at(Choices(EnStageChoices::Easy)) = param->easyPosition;
			m_choicesPositions.at(Choices(EnStageChoices::Normal)) = param->normalPosition;
			m_choicesPositions.at(Choices(EnStageChoices::Hard)) = param->hardPosition;
			m_textColor = param->textColor;

			m_isSetUp = true;
		}


		void StageSelectStatus::Update()
		{
			SetUp();
		}
	}
}