/**
 * @file PlayerStatus.cpp
 * @brief プレイヤーのステータスクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "PlayerStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		PlayerStatus::PlayerStatus()
		{
			// 外部ファイルを読み込み
			ParameterManager::Get().LoadParameter<MasterPlayerParameter>("Assets/parameter/player/PlayerParameter.json", [](const nlohmann::json& j, MasterPlayerParameter& parameter)
				{
					parameter.maxHp = j["maxHp"].get<int>();
					parameter.hp = j["hp"].get<int>();
					parameter.walkSpeed = j["walkSpeed"].get<float>();
					parameter.runSpeed = j["runSpeed"].get<float>();
					parameter.radius = j["radius"].get<float>();
					parameter.height = j["height"].get<float>();
				});
		}


		PlayerStatus::~PlayerStatus()
		{
			// 使用終了
			ParameterManager::Get().UnloadParameter<MasterPlayerParameter>();
		}


		void PlayerStatus::Setup()
		{
			// 読み込んだパラメーター取得
			const auto* parameter = ParameterManager::Get().GetParameter<MasterPlayerParameter>();
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}


		void PlayerStatus::Update()
		{
			const auto* parameter = ParameterManager::Get().GetParameter<MasterPlayerParameter>();
			m_maxHp = parameter->maxHp;
			m_hp = parameter->hp;
			m_walkSpeed = parameter->walkSpeed;
			m_runSpeed = parameter->runSpeed;
			m_radius = parameter->radius;
			m_height = parameter->height;
		}
	}
}