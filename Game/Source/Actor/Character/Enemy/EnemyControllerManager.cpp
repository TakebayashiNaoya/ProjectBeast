/**
 * @file NPCControllerManager.cpp
 * @brief NPCのコントローラーのマネージャー
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyController.h"
#include "EnemyControllerManager.h"


namespace app
{
	EnemyControllerManager* EnemyControllerManager::m_instance = nullptr;


	EnemyControllerManager::EnemyControllerManager()
	{

	}


	EnemyControllerManager::~EnemyControllerManager()
	{

	}


	void EnemyControllerManager::Register(actor::EnemyController* npc)
	{
		if (npc == nullptr) {
			return;
		}

		m_enemyControllerList.push_back(npc);
	}


	void EnemyControllerManager::UnRegister(actor::EnemyController* npc)
	{
		if (!npc) {
			return;
		}

		auto it = std::find(
			m_enemyControllerList.begin(),
			m_enemyControllerList.end(),
			npc
		);

		if (it != m_enemyControllerList.end())
		{
			m_enemyControllerList.erase(it);
		}
	}


	void EnemyControllerManager::Update()
	{
		for (auto& npc : m_enemyControllerList)
		{
			if (!npc)
			{
				continue;
			}
			npc->Update();
		}
	}
}