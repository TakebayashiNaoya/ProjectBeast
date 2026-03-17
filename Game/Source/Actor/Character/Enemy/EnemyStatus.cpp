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
			core::ParameterManager::Get()->LoadParameter<MasterEnemyParameter>("Assets/parameter/player/PlayerParameter.json", [](const nlohmann::json& j, MasterEnemyParameter& parameter)
				{
					parameter.maxHp = j["maxHp"].get<int>();
					parameter.hp = j["hp"].get<int>();
					parameter.walkSpeed = j["walkSpeed"].get<float>();
					parameter.runSpeed = j["runSpeed"].get<float>();
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();
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
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void EnemyStatus::Update()
		{
			Setup();
		}
	}
}