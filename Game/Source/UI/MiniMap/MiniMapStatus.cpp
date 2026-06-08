/**
 * @file MiniMapStatus.cpp
 * @brief MiniMapStatusクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "MiniMapParameter.h"
#include "MiniMapStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// ミニマップ専用JSONのパス。
			const char* JSON_PATH = "Assets/parameter/miniMap/MiniMapParameter.json";
		}


		MiniMapStatus::MiniMapStatus()
			: m_radius(0.0f)
			, m_limitDistance(0.0f)
			, m_mapCenterPos(Vector3::Zero)
		{
			core::ParameterManager::Get()->LoadParameter<MiniMapParameter>(JSON_PATH, [](const nlohmann::json& j, MiniMapParameter& param)
				{
					param.mapRadius = j["radius"].get<float>();
					param.mapLimitDistance = j["limitDis"].get<float>();
					param.mapCenterPos = util::JsonConverter::ToVector3(j["centerPos"]);
				});
		}


		MiniMapStatus::~MiniMapStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MiniMapParameter>();
		}


		void MiniMapStatus::SetUp()
		{
			const auto* param = core::ParameterManager::Get()->GetParameter<MiniMapParameter>();
			m_radius = param->mapRadius;
			m_limitDistance = param->mapLimitDistance;
			m_mapCenterPos = param->mapCenterPos;
		}


		void MiniMapStatus::Update()
		{
			SetUp();
		}
	}
}