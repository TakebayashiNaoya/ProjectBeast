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
					p.choicesPivot = app::util::JsonConverter::ToVector2(j, "choicesPivot");
					p.easyPosition = app::util::JsonConverter::ToVector3(j, "easyPosition");
					p.normalPosition = app::util::JsonConverter::ToVector3(j, "normalPosition");
					p.hardPosition = app::util::JsonConverter::ToVector3(j, "hardPosition");
					p.backPosition = app::util::JsonConverter::ToVector3(j, "backPosition");
					p.offSetPosition = app::util::JsonConverter::ToVector3(j, "offsetPosition");
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

			m_choicesPositions.at(static_cast<size_t>(EnStageChoices::Back)) = param->backPosition;
			m_choicesPositions.at(static_cast<size_t>(EnStageChoices::Easy)) = param->easyPosition;
			m_choicesPositions.at(static_cast<size_t>(EnStageChoices::Normal)) = param->normalPosition;
			m_choicesPositions.at(static_cast<size_t>(EnStageChoices::Hard)) = param->hardPosition;
			m_textColor = param->textColor;

			m_isSetUp = true;
		}


		void StageSelectStatus::Update()
		{
			SetUp();
		}
	}
}