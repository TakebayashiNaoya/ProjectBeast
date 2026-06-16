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
		}


		ChildPenguinStatus::ChildPenguinStatus()
		{
			/** 外部ファイルを読み込み */
			core::ParameterManager::Get()->LoadParameterBinary<MasterChildPenguinParameter>(
				PARAMETER_BINARY_FILE_PATH,
				[](std::istream& stream, MasterChildPenguinParameter& parameter)
				{
					// タイプ（インデックス）を読み込む
					int typeIndex = 0;
					stream.read(reinterpret_cast<char*>(&typeIndex), sizeof(int));

					// 範囲外アクセス防止のフェイルセーフ
					if (typeIndex < 0 || typeIndex >= static_cast<int>(EnChildPenguinType::Num))
					{
						typeIndex = 0;
					}

					// 共通パラメーターを読み込む
					stream.read(reinterpret_cast<char*>(&parameter.maxHp), sizeof(int));
					stream.read(reinterpret_cast<char*>(&parameter.hp), sizeof(int));
					stream.read(reinterpret_cast<char*>(&parameter.radius), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.height), sizeof(float));

					// タイプ別の個体差データを読み込む
					auto& td = parameter.typeData[typeIndex];

					stream.read(reinterpret_cast<char*>(&td.colorR), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.colorG), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.colorB), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.colorA), sizeof(float));

					stream.read(reinterpret_cast<char*>(&td.runSpeed.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.runSpeed.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.swimSpeed.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.swimSpeed.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.sneakSpeed.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.sneakSpeed.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.slideSpeed.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.slideSpeed.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.jumpPower.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.jumpPower.max), sizeof(float));

					stream.read(reinterpret_cast<char*>(&td.stopDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.stopDistance.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.walkDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.walkDistance.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.runDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.runDistance.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.joinDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.joinDistance.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.giveUpDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.giveUpDistance.max), sizeof(float));

					// やんちゃペンギン固有
					stream.read(reinterpret_cast<char*>(&td.roamTriggerDistance.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.roamTriggerDistance.max), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.roamRadius.min), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.roamRadius.max), sizeof(float));

					// おっちょこちょいペンギン固有
					stream.read(reinterpret_cast<char*>(&td.tripChancePerSec), sizeof(float));
					stream.read(reinterpret_cast<char*>(&td.slipChance), sizeof(float));

					// 世話焼きペンギン固有
					stream.read(reinterpret_cast<char*>(&td.interventionRange), sizeof(float));
				});
		}


		ChildPenguinStatus::~ChildPenguinStatus()
		{
			/** 使用終了 */
			core::ParameterManager::Get()->UnloadParameter<MasterChildPenguinParameter>();
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