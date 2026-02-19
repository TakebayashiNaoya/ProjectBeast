/**
 * @file NPCControllerManager.cpp
 * @brief NPCのコントローラーのマネージャー
 * @author 立山
 */
#include "stdafx.h"
#include "NPCController.h"
#include "NPCControllerManager.h"


namespace app
{
	NPCControllerManager* NPCControllerManager::m_instance = nullptr;


	NPCControllerManager::NPCControllerManager()
	{

	}


	NPCControllerManager::~NPCControllerManager()
	{

	}


	void NPCControllerManager::Register(actor::NPCController* npc)
	{
		if (npc == nullptr) {
			return;
		}

		m_npcControllerList.push_back(npc);
	}


	void NPCControllerManager::UnRegister(actor::NPCController* npc)
	{
		if (!npc) {
			return;
		}

		auto it = std::find(
			m_npcControllerList.begin(),
			m_npcControllerList.end(),
			npc
		);

		if (it != m_npcControllerList.end())
		{
			m_npcControllerList.erase(it);
		}
	}


	void NPCControllerManager::Update()
	{
		for (auto& npc : m_npcControllerList)
		{
			if (!npc)
			{
				continue;
			}
			npc->Update();
		}
	}
}