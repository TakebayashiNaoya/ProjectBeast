/**
 * @file EnemyStatus.cpp
 * @brief エネミーのステータス
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyParameter.h"
#include "EnemyStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		EnemyStatus::EnemyStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterEnemyParameter>("Assets/parameter/character/enemy/EnemyParameter.json", [](const nlohmann::json& j, MasterEnemyParameter& parameter)
				{
					parameter.walkSpeed = j["walkSpeed"].get<float>();
					parameter.runSpeed = j["runSpeed"].get<float>();
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();
					parameter.swimSpeed = j["swimSpeed"].get<float>();
					parameter.maxEat = j["maxEat"].get<int>();
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