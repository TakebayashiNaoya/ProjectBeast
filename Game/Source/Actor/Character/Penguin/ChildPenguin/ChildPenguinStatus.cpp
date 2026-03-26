/**
 * @file ChildPenguinStatus.cpp
 * @brief 子ペンギンのステータスクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 子ペンギンのパラメーターのファイルパス */
			const char* PARAMETER_FILE_PATH = "Assets/parameter/character/penguin/childPenguin/ChildPenguinParameter.json";
		}


		ChildPenguinStatus::ChildPenguinStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterChildPenguinParameter>(PARAMETER_FILE_PATH, [](const nlohmann::json& j, MasterChildPenguinParameter& parameter)
				{
					parameter.maxHp = j["maxHp"].get<int>();
					parameter.hp = j["hp"].get<int>();
					parameter.walkSpeed = j["walkSpeed"].get<float>();
					parameter.runSpeed = j["runSpeed"].get<float>();
					parameter.swimSpeed = j["swimSpeed"].get<float>();
					parameter.sneakSpeed = j["sneakSpeed"].get<float>();
					parameter.slideSpeed = j["slideSpeed"].get<float>();
					parameter.jumpPower = j["jumpPower"].get<float>();
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();
				});
		}


		ChildPenguinStatus::~ChildPenguinStatus()
		{
			// 使用終了
			core::ParameterManager::Get()->UnloadParameter<MasterChildPenguinParameter>();
		}


		void ChildPenguinStatus::Setup()
		{
			// 読み込んだパラメーター取得
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterChildPenguinParameter>();
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_swimSpeed = parameter->swimSpeed;
			m_sneakSpeed = parameter->sneakSpeed;
			m_slideSpeed = parameter->slideSpeed;
			m_jumpPower = parameter->jumpPower;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void ChildPenguinStatus::Update()
		{
			Setup();
		}
	}
}