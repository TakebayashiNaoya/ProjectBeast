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
			constexpr const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/daddyPenguin/DaddyPenguinParameter.bin";
		}


		DaddyPenguinStatus::DaddyPenguinStatus()
			: m_enableCommandRange(0.0f)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameterBinary<MasterDaddyPenguinParameter>(
				PARAMETER_BINARY_FILE_PATH
			);
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
			m_jumpStaminaMax = parameter->jumpStaminaMax;                     // ← 追加
			m_jumpStaminaRecoverSpeed = parameter->jumpStaminaRecoverSpeed;   // ← 追加
			m_slideStaminaMax = parameter->slideStaminaMax;                   // ← 追加
			m_slideStaminaDecreaseSpeed = parameter->slideStaminaDecreaseSpeed; // ← 追加
			m_slideStaminaRecoverSpeed = parameter->slideStaminaRecoverSpeed; // ← 追加
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