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
			const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/daddyPenguin/DaddyPenguinParameter.bin";
		}


		DaddyPenguinStatus::DaddyPenguinStatus()
			: m_enableCommandRange(0.0f)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameterBinary<MasterDaddyPenguinParameter>(
				PARAMETER_BINARY_FILE_PATH,
				[](std::istream& stream, MasterDaddyPenguinParameter& parameter)
				{
					stream.read(reinterpret_cast<char*>(&parameter.maxHp), sizeof(int));
					stream.read(reinterpret_cast<char*>(&parameter.hp), sizeof(int));
					stream.seekg(sizeof(float), std::ios::cur);
					stream.read(reinterpret_cast<char*>(&parameter.runSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.swimSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.sneakSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.slideSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.jumpPower), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.radius), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.height), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.enableCommandRange), sizeof(float));
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
			m_runSpeed = parameter->runSpeed;
			m_swimSpeed = parameter->swimSpeed;
			m_sneakSpeed = parameter->sneakSpeed;
			m_slideSpeed = parameter->slideSpeed;
			m_jumpPower = parameter->jumpPower;
			m_radius = parameter->radius;
			m_height = parameter->height;
			m_enableCommandRange = parameter->enableCommandRange;
		}


		void DaddyPenguinStatus::Update()
		{
			Setup();
		}
	}
}