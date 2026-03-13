/**
 * @file DaddyPenguinStatus.cpp
 * @brief 親ペンギンのステータスクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguinParameter.h"
#include "DaddyPenguinStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 親ペンギンのパラメーターのファイルパス */
			const char* PARAMETER_FILE_PATH = "Assets/parameter/character/penguin/daddyPenguin/DaddyPenguinParameter.json";
		}


		DaddyPenguinStatus::DaddyPenguinStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterDaddyPenguinParameter>(PARAMETER_FILE_PATH, [](const nlohmann::json& j, MasterDaddyPenguinParameter& parameter)
				{
					parameter.maxHp = j["maxHp"].get<int>();
					parameter.hp = j["hp"].get<int>();
					parameter.walkSpeed = j["walkSpeed"].get<float>();
					parameter.runSpeed = j["runSpeed"].get<float>();
					parameter.swimSpeed = j["swimSpeed"].get<float>();
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();
				});
		}


		DaddyPenguinStatus::~DaddyPenguinStatus()
		{
			// 使用終了
			core::ParameterManager::Get()->UnloadParameter<MasterDaddyPenguinParameter>();
		}


		void DaddyPenguinStatus::Setup()
		{
			// 読み込んだパラメーター取得
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterDaddyPenguinParameter>();
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_swimSpeed = parameter->swimSpeed;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void DaddyPenguinStatus::Update()
		{
			Setup();
		}
	}
}