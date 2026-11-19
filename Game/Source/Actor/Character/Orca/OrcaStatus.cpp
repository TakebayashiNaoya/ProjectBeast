/**
 * @file OrcaStatus.cpp
 * @brief シャチのステータス
 */
#include "stdafx.h"
#include "OrcaStatus.h"
#include "OrcaParameter.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/orca/OrcaParameter.bin";
		}


		OrcaStatus::OrcaStatus()
		{
			core::ParameterManager::Get()->LoadParameterBinary<MasterOrcaParameter>(PARAMETER_BINARY_FILE_PATH);
		}


		OrcaStatus::~OrcaStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterOrcaParameter>();
		}


		void OrcaStatus::Setup()
		{
			m_parameter = core::ParameterManager::Get()->GetParameter<MasterOrcaParameter>();
			K2_ASSERT(m_parameter != nullptr, "OrcaParameter.bin を読み込めません。Tools/ParameterConvert.py を実行してください");
			if (m_parameter == nullptr) return;

			m_runSpeed = m_parameter->runSpeed;
			m_swimSpeed = m_parameter->swimSpeed;
			m_radius = m_parameter->radius;
			m_height = m_parameter->height;

			// 初回だけスタミナを満タンにする（EnemyStatus::Setup と同じ）
			if (m_stamina == UNINITIALIZED_STAMINA)
			{
				m_stamina = m_parameter->maxStamina;
			}
			// ホットリロードで上限を下げたときに丸める
			if (m_stamina > m_parameter->maxStamina)
			{
				m_stamina = m_parameter->maxStamina;
			}
		}


		void OrcaStatus::Update()
		{
			Setup();
		}


		int OrcaStatus::GetMaxEat() const
		{
			if (m_parameter == nullptr) return 1;
			return max(1, static_cast<int>(m_parameter->maxEat + 0.5f));
		}


		void OrcaStatus::DecreaseStamina(float amount)
		{
			m_stamina -= amount;
			if (m_stamina < 0.0f)
			{
				m_stamina = 0.0f;
			}
		}


		void OrcaStatus::FullRecoverStamina()
		{
			if (m_parameter == nullptr) return;
			m_stamina = m_parameter->maxStamina;
		}
	}
}
