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
		namespace
		{
			constexpr const char* STAGE_SELECT_PARAMETER_PATH =
				"Assets/parameter/UI/stageSelect/StageSelectParameter.json";
		}




		StageSelectStatus::StageSelectStatus()
			: m_inputInterval(0.0f)
			, m_inputThreshold(0.0f)
			, m_choicesYOffset(0.0f)
			, m_choicesPositionX{}
			, m_buttonXOffset(0.0f)
			, m_buttonYOffset(0.0f)
			, m_buttonPositionX{}
			, m_textBGColor(Vector4::White)
			, m_buttonBGPosition(Vector3::Zero)
			, m_stageSelectPosition(Vector3::Zero)
			, m_choicesTextColor(Vector4::White)
		{}


		StageSelectStatus::~StageSelectStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterStageSelectParameter>();
		}


		void StageSelectStatus::SetUp()
		{
			core::ParameterManager::Get()->LoadParameter<MasterStageSelectParameter>(
				STAGE_SELECT_PARAMETER_PATH,
				[](const nlohmann::json& j, MasterStageSelectParameter& p)
				{
					p.inputInterval = app::util::JsonConverter::ToFloat(j, "inputInterval");
					p.inputThreshold = app::util::JsonConverter::ToFloat(j, "inputThreshold");

					p.stageSelectPosition = app::util::JsonConverter::ToVector3(j, "stageSlectPosition");

					p.choicesYOffset = app::util::JsonConverter::ToFloat(j, "choicesYOffset");
					p.tutorialPositionX = app::util::JsonConverter::ToFloat(j, "tutorialPositionX");
					p.easyPositionX = app::util::JsonConverter::ToFloat(j, "easyPositionX");
					p.normalPositionX = app::util::JsonConverter::ToFloat(j, "normalPositionX");
					p.hardPositionX = app::util::JsonConverter::ToFloat(j, "hardPositionX");
					p.choicesTextColor = app::util::JsonConverter::ToVector4(j, "choicesTextColor");


					p.buttonXOffset = app::util::JsonConverter::ToFloat(j, "buttonXOffset");
					p.buttonYOffset = app::util::JsonConverter::ToFloat(j, "buttonYOffset");
					p.backButtonPositionX = app::util::JsonConverter::ToFloat(j, "backButtonPositionX");
					p.decideButtonPositionX = app::util::JsonConverter::ToFloat(j, "decideButtonPositionX");
					p.selectButtonPositionX = app::util::JsonConverter::ToFloat(j, "selectButtonPositionX");
					p.buttonBGPosition = app::util::JsonConverter::ToVector3(j, "buttonBGPosition");


					p.textBGColor = app::util::JsonConverter::ToVector4(j, "textBGColor");
				}
			);


			auto* param = core::ParameterManager::Get()->GetParameter<MasterStageSelectParameter>();
			if (!param) return;


			m_inputInterval = param->inputInterval;
			m_inputThreshold = param->inputThreshold;

			m_stageSelectPosition = param->stageSelectPosition;

			m_choicesYOffset = param->choicesYOffset;
			m_choicesPositionX.at(static_cast<size_t>(EnStageChoices::Tutorial)) = param->tutorialPositionX;
			m_choicesPositionX.at(static_cast<size_t>(EnStageChoices::Easy)) = param->easyPositionX;
			m_choicesPositionX.at(static_cast<size_t>(EnStageChoices::Normal)) = param->normalPositionX;
			m_choicesPositionX.at(static_cast<size_t>(EnStageChoices::Hard)) = param->hardPositionX;

			m_buttonXOffset = param->buttonXOffset;
			m_buttonYOffset = param->buttonYOffset;
			m_buttonPositionX.at(static_cast<size_t>(EnStageButtonTypes::Back)) = param->backButtonPositionX;
			m_buttonPositionX.at(static_cast<size_t>(EnStageButtonTypes::Decide)) = param->decideButtonPositionX;
			m_buttonPositionX.at(static_cast<size_t>(EnStageButtonTypes::Select)) = param->selectButtonPositionX;

			m_choicesTextColor = param->choicesTextColor;
			m_textBGColor = param->textBGColor;
			m_buttonBGPosition = param->buttonBGPosition;


			m_isSetUp = true;
		}


		void StageSelectStatus::Update()
		{
			if (m_isSetUp) return;
			SetUp();
		}
	}
}