/**
 * @file IglooManager.cpp
 * @brief かまくらを管理するクラス
 */
#include "stdafx.h"

#include "IglooManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinAIController.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"


namespace app
{
	namespace actor {

		IglooManager* IglooManager::m_instance = nullptr;

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


		void IglooManager::EjectAllPenguins(const Vector3& iglooPosition)
		{
			for (ChildPenguin* penguin : m_insidePenguinList)
			{
				if (penguin != nullptr && penguin->GetAIController() != nullptr)
				{
					// AIコントローラーの強制排出関数を呼ぶ
					penguin->GetAIController()->ForceEjectFromIgloo(iglooPosition);
				}
			}
			ClearPenguins();


			// 親ペンギンの強制排出処理
			if (m_insideDaddy != nullptr)
			{
				Vector3 daddySpawnPos = iglooPosition;
				daddySpawnPos.x += 50.0f;
				daddySpawnPos.y += 50.0f; // 少し上空から落とす

				m_insideDaddy->SetPosition(daddySpawnPos);
				m_insideDaddy->GetStateMachine()->SetIsInsideIgloo(false); // ステート強制離脱

				// 外に出したので nullptr に戻す
				m_insideDaddy = nullptr;
			}
		}

	}
}