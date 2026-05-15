/**
 * @file EnemyStatus.cpp
 * @brief エネミーのステータス
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyParameter.h"
#include "EnemyStatus.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace actor
	{
		EnemyStatus::EnemyStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterEnemyParameter>("Assets/parameter/character/enemy/EnemyParameter.json", [](const nlohmann::json& j, MasterEnemyParameter& parameter)
				{
					parameter.walkSpeed = util::JsonConverter::ToFloat(j, "walkSpeed");
					parameter.runSpeed = util::JsonConverter::ToFloat(j, "runSpeed");
					parameter.radius = util::JsonConverter::ToFloat(j, "radius");
					parameter.height = util::JsonConverter::ToFloat(j, "height");
					parameter.swimSpeed = util::JsonConverter::ToFloat(j, "swimSpeed");
					parameter.maxEat = util::JsonConverter::ToInt(j, "maxEat");
				});
		}


		EnemyStatus::~EnemyStatus()
		{
			// 使用終了
			core::ParameterManager::Get()->UnloadParameter<MasterEnemyParameter>();
		}


		void EnemyStatus::Setup()
		{
			// 読み込んだパラメーター取得
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterEnemyParameter>();
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_radius = parameter->radius;
			m_height = parameter->height;
			m_swimSpeed = parameter->swimSpeed;
			m_maxEat = parameter->maxEat;
		}


		void EnemyStatus::Update()
		{
			Setup();
		}
	}
}