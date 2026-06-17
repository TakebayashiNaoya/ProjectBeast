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
			const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/childPenguin/ChildPenguinParameter.bin";
			/** 子ペンギンのタイプ別パラメーターのファイルパス */
			const char* TYPE_PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/childPenguin/ChildPenguinTypeParameter.bin";
		}


		ChildPenguinStatus::ChildPenguinStatus()
		{
			/** 外部ファイルを読み込み */
			core::ParameterManager::Get()->LoadParameterBinary<MasterChildPenguinParameter>(
				PARAMETER_BINARY_FILE_PATH
			);
			core::ParameterManager::Get()->LoadParameterBinary<MasterChildPenguinTypeParameter>(
				TYPE_PARAMETER_BINARY_FILE_PATH
			);
		}


		ChildPenguinStatus::~ChildPenguinStatus()
		{
			/** 使用終了 */
			core::ParameterManager::Get()->UnloadParameter<MasterChildPenguinParameter>();
			core::ParameterManager::Get()->UnloadParameter<MasterChildPenguinTypeParameter>();
		}


		void ChildPenguinStatus::Setup()
		{
			/** 読み込んだパラメーター取得 */
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterChildPenguinParameter>();
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void ChildPenguinStatus::Update()
		{
			/** 個体値がロックされている間はホットリロードによる上書きをスキップする */
			if (!m_isIndividualValueLocked)
			{
				Setup();
			}
		}


		void ChildPenguinStatus::SetIndividualValues(
			float runSpeed,
			float swimSpeed,
			float sneakSpeed,
			float slideSpeed,
			float jumpPower)
		{
			m_runSpeed = runSpeed;
			m_swimSpeed = swimSpeed;
			m_sneakSpeed = sneakSpeed;
			m_slideSpeed = slideSpeed;
			m_jumpPower = jumpPower;
			m_isIndividualValueLocked = true;
		}
	}
}