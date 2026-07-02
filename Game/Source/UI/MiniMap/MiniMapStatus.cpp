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
			const char* JSON_PATH = "Assets/parameter/UI/miniMap/MiniMapParameter.json";
		}


		MiniMapStatus::MiniMapStatus()
			: m_radius(0.0f)
			, m_limitDistance(0.0f)
			, m_mapCenterPos(Vector3::Zero)
		{
			// アイコン種別に対応するJSONキー (EnMiniMapIconType の順番と一致させる)
			struct IconKeys {
				const char* nameKey;
				const char* widthKey;
				const char* heightKey;
			};
			static constexpr std::array<IconKeys, static_cast<uint8_t>(EnMiniMapIconType::Num)> ICON_KEYS = { {
				{ "seriousName",  "childPenWidth",   "childPenHeight"   },
				{ "clingyName",   "childPenWidth",   "childPenHeight"   },
				{ "naughtyName",  "childPenWidth",   "childPenHeight"   },
				{ "clumsyName",   "childPenWidth",   "childPenHeight"   },
				{ "caringName",   "childPenWidth",   "childPenHeight"   },
				{ "bearNestName", "bearNestWidth",   "bearNestHeight"   },
				{ "bearName",     "bearWidth",       "bearHeight"       },
				{ "whirlpoolName","whirlpoolWidth",  "whirlpoolHeight"  },
				{ "iglooName",    "iglooWidth",      "iglooHeight"      },
			} };



			core::ParameterManager::Get()->LoadParameter<MiniMapParameter>(JSON_PATH, [](const nlohmann::json& j, MiniMapParameter& param)
				{
					using JC = util::JsonConverter;

					param.mapRadius = JC::ToFloat(j, "radius");
					param.mapLimitDistance = JC::ToFloat(j, "limitDis");
					param.mapCenterPos = JC::ToVector3(j, "centerPos");
					param.initPosition = JC::ToVector3(j, "initPosition");
					param.initScale = JC::ToVector3(j, "initScale");
					param.initRotation = JC::ToRotation(j, "initRotationDeg");
					param.initColor = JC::ToVector4(j, "initColor");
					param.daddyInfo.path = JC::ToString(j, "daddyName");
					param.daddyInfo.width = JC::ToFloat(j, "daddyWidth");
					param.daddyInfo.height = JC::ToFloat(j, "daddyHeight");

					for (uint8_t i = 0; i < static_cast<uint8_t>(EnMiniMapIconType::Num); i++)
					{
						const auto& keys = ICON_KEYS[i];
						auto& info = param.iconInitializeInfos.at(i);
						info.path = JC::ToString(j, keys.nameKey);
						info.width = JC::ToFloat(j, keys.widthKey);
						info.height = JC::ToFloat(j, keys.heightKey);
					}
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

			m_daddyInfo = param->daddyInfo;
			m_iconInitializeInfos = param->iconInitializeInfos;

			m_initPosition = param->initPosition;
			m_initScale = param->initScale;
			m_initRotation = param->initRotation;
			m_initColor = param->initColor;
		}


		void MiniMapStatus::Update()
		{
			SetUp();
		}
	}
}