/**
 * @file CPReactionStatus.cpp
 * @brief CPReactionのステータス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionStatus.h"
#include "MasterCPReactionParameter.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{
		CPReactionStatus::CPReactionStatus()
			: m_swayTime(0.0f)
			, m_iconOffsetY(0.0f)
			, m_drawableDistance(0.0f)
			, m_speechBubbleOffset(Vector3::Zero)
			, m_troubleReactionOffset(Vector3::Zero)
			, m_happyReactionOffset(Vector3::Zero)
			, m_seriousReactionColor(Vector4::White)
			, m_clingyReactionColor(Vector4::White)
			, m_naughtyReactionColor(Vector4::White)
			, m_clumsyReactionColor(Vector4::White)
			, m_caringReactionColor(Vector4::White)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterCPReactionParameter>("Assets/parameter/UI/cpReaction/CPReactionParameter.json", [](const nlohmann::json& j, MasterCPReactionParameter& parameter)
				{
					parameter.swayTime = util::JsonConverter::ToFloat(j, "swayTime");
					parameter.iconOffsetY = util::JsonConverter::ToFloat(j, "iconOffsetY");
					parameter.drawableDistance = util::JsonConverter::ToFloat(j, "drawableDistance");
					parameter.speechBubbleOffset = util::JsonConverter::ToVector3(j, "speechBubbleOffset");
					parameter.troubleReactionOffset = util::JsonConverter::ToVector3(j, "troubleReactionOffset");
					parameter.happyReactionOffset = util::JsonConverter::ToVector3(j, "happyReactionOffset");
					parameter.seriousReactionColor = util::JsonConverter::ToVector4(j, "seriousReactionColor");
					parameter.clingyReactionColor = util::JsonConverter::ToVector4(j, "clingyReactionColor");
					parameter.naughtyReactionColor = util::JsonConverter::ToVector4(j, "naughtyReactionColor");
					parameter.clumsyReactionColor = util::JsonConverter::ToVector4(j, "clumsyReactionColor");
					parameter.caringReactionColor = util::JsonConverter::ToVector4(j, "caringReactionColor");
				});
		}


		CPReactionStatus::~CPReactionStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterCPReactionParameter>();
		}


		void CPReactionStatus::SetUp()
		{
			// 読み込んだパラメーター取得
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterCPReactionParameter>();
			if (!parameter) return;
			m_swayTime = parameter->swayTime;
			m_iconOffsetY = parameter->iconOffsetY;
			m_drawableDistance = parameter->drawableDistance;
			m_speechBubbleOffset = parameter->speechBubbleOffset;
			m_troubleReactionOffset = parameter->troubleReactionOffset;
			m_happyReactionOffset = parameter->happyReactionOffset;
			m_seriousReactionColor = parameter->seriousReactionColor;
			m_clingyReactionColor = parameter->clingyReactionColor;
			m_naughtyReactionColor = parameter->naughtyReactionColor;
			m_clumsyReactionColor = parameter->clumsyReactionColor;
			m_caringReactionColor = parameter->caringReactionColor;
		}


		void CPReactionStatus::Update()
		{
			SetUp();
		}
	}
}