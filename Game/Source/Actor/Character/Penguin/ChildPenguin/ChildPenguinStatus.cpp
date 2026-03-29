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
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();

					// タイプ別個体差パラメーター範囲を読み込む
					const auto& jRange = j["randomRanges"];
					const int typeIndex = j["type"].get<int>();
					auto& td = parameter.typeData[typeIndex];
					td.colorR = j["color"][0].get<float>();
					td.colorG = j["color"][1].get<float>();
					td.colorB = j["color"][2].get<float>();
					td.colorA = j["color"][3].get<float>();
					td.runSpeed.min = jRange["runSpeed"][0].get<float>();
					td.runSpeed.max = jRange["runSpeed"][1].get<float>();
					td.swimSpeed.min = jRange["swimSpeed"][0].get<float>();
					td.swimSpeed.max = jRange["swimSpeed"][1].get<float>();
					td.sneakSpeed.min = jRange["sneakSpeed"][0].get<float>();
					td.sneakSpeed.max = jRange["sneakSpeed"][1].get<float>();
					td.slideSpeed.min = jRange["slideSpeed"][0].get<float>();
					td.slideSpeed.max = jRange["slideSpeed"][1].get<float>();
					td.jumpPower.min = jRange["jumpPower"][0].get<float>();
					td.jumpPower.max = jRange["jumpPower"][1].get<float>();
					td.stopDistance.min = jRange["stopDistance"][0].get<float>();
					td.stopDistance.max = jRange["stopDistance"][1].get<float>();
					td.walkDistance.min = jRange["walkDistance"][0].get<float>();
					td.walkDistance.max = jRange["walkDistance"][1].get<float>();
					td.runDistance.min = jRange["runDistance"][0].get<float>();
					td.runDistance.max = jRange["runDistance"][1].get<float>();
					td.joinDistance.min = jRange["joinDistance"][0].get<float>();
					td.joinDistance.max = jRange["joinDistance"][1].get<float>();
					td.giveUpDistance.min = jRange["giveUpDistance"][0].get<float>();
					td.giveUpDistance.max = jRange["giveUpDistance"][1].get<float>();
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
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void ChildPenguinStatus::Update()
		{
			// 個体値がロックされている間はホットリロードによる上書きをスキップする
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