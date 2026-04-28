/**
 * @file InGameAchievementMenu.cpp
 * @brief インゲーム中にアチーブメントの一覧と達成状況を表示するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "InGameAchievementMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		InGameAchievementMenu::InGameAchievementMenu()
		{}


		void InGameAchievementMenu::InitializeLogic()
		{
			m_entries.clear();

			auto* am = app::achievement::AchievementManager::GetInstance();
			if (!am) return;

			auto achieveList = am->GetAllAchievements();
			if (achieveList.empty()) return;

			for (size_t i = 0; i < achieveList.size(); ++i)
			{
				auto* achieve = achieveList[i];
				if (!achieve) continue;

				// JSONのelementsで定義したキー名でUIを取得する
				const std::string checkKeyName = "AchieveCheck_" + std::to_string(i);
				auto* checkIcon = GetUI<UIIcon>(Hash32(checkKeyName.c_str()));

				// 達成済みなら最初から表示、未達成なら非表示
				if (checkIcon)
				{
					checkIcon->m_isDraw = achieve->IsAchieved();
				}

				AchievementUIEntry entry;
				entry.achieve = achieve;
				entry.checkIcon = checkIcon;
				entry.wasAchieved = achieve->IsAchieved();
				m_entries.push_back(entry);
			}
		}


		void InGameAchievementMenu::Update()
		{
			// 毎フレーム達成状態を確認し、未達成→達成の瞬間にチェックアイコンを表示する
			for (auto& entry : m_entries)
			{
				if (!entry.achieve || !entry.checkIcon) continue;

				const bool isAchieved = entry.achieve->IsAchieved();

				if (isAchieved && !entry.wasAchieved)
				{
					entry.checkIcon->m_isDraw = true;
				}

				entry.wasAchieved = isAchieved;
			}

			// Canvasの更新
			MenuBase::Update();
		}
	}
}