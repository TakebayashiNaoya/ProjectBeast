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
		namespace
		{
			constexpr int MAX_ACHIEVEMENT_ROWS = 10;
		}


		InGameAchievementMenu::InGameAchievementMenu()
		{}


		void InGameAchievementMenu::InitializeLogic()
		{
			m_entries.clear();

			auto* am = app::achievement::AchievementManager::GetInstance();
			if (!am) return;

			auto achieveList = am->GetAllAchievements();

			for (int i = 0; i < MAX_ACHIEVEMENT_ROWS; ++i)
			{
				const std::string idxStr    = std::to_string(i);
				auto* nameText  = GetUI<UIText> (Hash32(("AchieveName_"  + idxStr).c_str()));
				auto* boxIcon   = GetUI<UIIcon> (Hash32(("AchieveBox_"   + idxStr).c_str()));
				auto* checkIcon = GetUI<UIIcon> (Hash32(("AchieveCheck_" + idxStr).c_str()));

				if (i < static_cast<int>(achieveList.size()))
				{
					auto* achieve = achieveList[i];
					// JSONのelementsで定義したキー名でUIを取得する
					if (nameText)
					{
						nameText->SetText(achieve->GetDescription());
						nameText->m_isDraw = true;
					}
					if (boxIcon)   boxIcon->m_isDraw   = true;
					if (checkIcon) checkIcon->m_isDraw = achieve->IsAchieved();

					AchievementUIEntry entry;
					entry.achieve     = achieve;
					entry.nameText    = nameText;
					entry.boxIcon     = boxIcon;
					entry.checkIcon   = checkIcon;
					entry.wasAchieved = achieve->IsAchieved();
					m_entries.push_back(entry);
				}
				else
				{
					if (nameText)  nameText->m_isDraw  = false;
					if (boxIcon)   boxIcon->m_isDraw   = false;
					if (checkIcon) checkIcon->m_isDraw = false;
				}
			}
		}


		void InGameAchievementMenu::Update()
		{
			auto* am = app::achievement::AchievementManager::GetInstance();
			if (am && am->GetReloadVersion() != m_lastReloadVersion)
			{
				m_lastReloadVersion = am->GetReloadVersion();
				InitializeLogic();
			}

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
