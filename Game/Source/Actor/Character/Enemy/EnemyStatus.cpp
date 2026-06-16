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
		namespace
		{
			const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/enemy/EnemyParameter.bin";
		}

		EnemyStatus::EnemyStatus()
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameterBinary<MasterEnemyParameter>(
				PARAMETER_BINARY_FILE_PATH,
				[](std::istream& stream, MasterEnemyParameter& parameter)
				{
					stream.seekg(sizeof(int) * 2, std::ios::cur);
					stream.read(reinterpret_cast<char*>(&parameter.walkSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.runSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.radius), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.height), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.swimSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.maxEat), sizeof(int));
					stream.read(reinterpret_cast<char*>(&parameter.maxStamina), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.staminaDrainRate), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.lostChaseDistance), sizeof(float));
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

			// 初回呼び出し時のみ最大スタミナをセットする
			// （マジックナンバー -1.0f を UNINITIALIZED_STAMINA に変更）
			if (m_stamina == UNINITIALIZED_STAMINA)
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