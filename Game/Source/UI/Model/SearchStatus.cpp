/**
 * @file SearchStatus.cpp
 * @brief PB追跡・索敵専用ステータスクラス
 */
#include "stdafx.h"
#include "SearchParameter.h"
#include "SearchStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// JSONファイルパス。
			const char* JSON_PATH = "Assets/parameter/search/SearchParameter.json";
		}


		SearchStatus::SearchStatus()
			: m_offsetValueY(0.0f)
			, m_dotValue(0.0f)
			, m_iconPosX(0.0f)
			, m_iconPosZ(0.0f)
			, m_offsetA(Vector3::Zero)
			, m_offsetB(Vector3::Zero)
		{
			core::ParameterManager::Get()->LoadParameter<SearchParameter>(JSON_PATH, [](const nlohmann::json& j, SearchParameter& param)
				{
					param.offsetValueY = j["offsetValueY"].get<float>();
					param.dotValue = j["dotValue"].get<float>();
					param.iconPosX = j["iconPosX"].get<float>();
					param.iconPosZ = j["iconPosZ"].get<float>();
					param.offsetA = util::JsonConverter::ToVector3(j["offsetA"]);
					param.offsetB = util::JsonConverter::ToVector3(j["offsetB"]);
				});
		}


		SearchStatus::~SearchStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<SearchParameter>();
		}


		void SearchStatus::SetUp()
		{
			const auto* param = core::ParameterManager::Get()->GetParameter<SearchParameter>();
			m_offsetValueY = param->offsetValueY;
			m_dotValue = param->dotValue;
			m_iconPosX = param->iconPosX;
			m_iconPosZ = param->iconPosZ;
			m_offsetA = param->offsetA;
			m_offsetB = param->offsetB;
		}


		void SearchStatus::Update()
		{
			SetUp();
		}
	}
}