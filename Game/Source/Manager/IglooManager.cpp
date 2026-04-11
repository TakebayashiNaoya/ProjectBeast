/**
 * @file IglooManager.cpp
 * @brief かまくらを管理するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "IglooManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"


namespace app
{
	namespace actor {

		void IglooManager::AddPenguin(ChildPenguin* penguin)
		{
			// 重複登録を防ぐ
			auto it = std::find(m_insidePenguinList.begin(), m_insidePenguinList.end(), penguin);
			if (it == m_insidePenguinList.end())
			{
				m_insidePenguinList.push_back(penguin);
			}
		}

		void IglooManager::ClearPenguins()
		{
			m_insidePenguinList.clear();
		}

	}
}