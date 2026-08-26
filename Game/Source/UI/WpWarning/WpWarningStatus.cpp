/**
 * @file WpWarningStatus.cpp
 * @brief WpWarningのステータス
 */
#include "stdafx.h"
#include "WpWarningStatus.h"

#include "MasterWpWarningParameter.h"

#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{

		WpWarningStatus::WpWarningStatus()
			: m_offsetY(0.0f)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterWpWarningParameter>("Assets/parameter/UI/wpWarning/WpWarningParameter.json", [](const nlohmann::json& j, MasterWpWarningParameter& parameter)
				{
					parameter.offsetY = util::JsonConverter::ToFloat(j, "offsetY");
				});
		}


		WpWarningStatus::~WpWarningStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterWpWarningParameter>();
		}


		void WpWarningStatus::SetUp()
		{
			auto* parameter = core::ParameterManager::Get()->GetParameter<MasterWpWarningParameter>();

			if (!parameter) return;

			m_offsetY = parameter->offsetY;
		}


		void WpWarningStatus::Update()
		{
			SetUp();
		}
	}
}