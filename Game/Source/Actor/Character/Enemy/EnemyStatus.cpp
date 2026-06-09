/**
 * @file EnemyStatus.cpp
 * @brief エネミーのステータス
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyParameter.h"
#include "EnemyStatus.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace actor
	{
		EnemyStatus::EnemyStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterEnemyParameter>("Assets/parameter/character/enemy/EnemyParameter.json", [](const nlohmann::json& j, MasterEnemyParameter& parameter)
				{
					parameter.walkSpeed = util::JsonConverter::ToFloat(j, "walkSpeed");
					parameter.runSpeed = util::JsonConverter::ToFloat(j, "runSpeed");
					parameter.radius = util::JsonConverter::ToFloat(j, "radius");
					parameter.height = util::JsonConverter::ToFloat(j, "height");
					parameter.swimSpeed = util::JsonConverter::ToFloat(j, "swimSpeed");
					parameter.maxEat = util::JsonConverter::ToInt(j, "maxEat");
					parameter.maxStamina = util::JsonConverter::ToFloat(j, "maxStamina");
					parameter.staminaDrainRate = util::JsonConverter::ToFloat(j, "staminaDrainRate");
					parameter.lostChaseDistance = util::JsonConverter::ToFloat(j, "lostChaseDistance");
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
			m_maxStamina = parameter->maxStamina;
			m_staminaDrainRate = parameter->staminaDrainRate;
			m_lostChaseDistance = parameter->lostChaseDistance;

			// 初回呼び出し時（-1.0f の時）のみ最大スタミナをセットする
			if (m_stamina < 0.0f)
			{
				m_stamina = m_maxStamina;
			}

			// ホットリロードで json の maxStamina を減らした場合の対策
			// 現在のスタミナが変更後の最大値を超えていたら丸める
			if (m_stamina > m_maxStamina)
			{
				m_stamina = m_maxStamina;
			}
		}


		void EnemyStatus::Update()
		{
			Setup();
		}
	}
}