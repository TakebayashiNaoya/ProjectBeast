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

			if (!m_basePositionsCaptured)
			{
				m_baseNamePos.resize(MAX_ACHIEVEMENT_ROWS);
				m_baseBoxPos.resize(MAX_ACHIEVEMENT_ROWS);
				m_baseCheckPos.resize(MAX_ACHIEVEMENT_ROWS);

				for (int i = 0; i < MAX_ACHIEVEMENT_ROWS; ++i)
				{
					const std::string idxStr = std::to_string(i);
					if (auto* t = GetUI<UIText>(Hash32(("AchieveName_" + idxStr).c_str())))
						m_baseNamePos[i] = t->m_transform.m_localTransform.m_position;
					if (auto* b = GetUI<UIIcon>(Hash32(("AchieveBox_" + idxStr).c_str())))
						m_baseBoxPos[i] = b->m_transform.m_localTransform.m_position;
					if (auto* c = GetUI<UIIcon>(Hash32(("AchieveCheck_" + idxStr).c_str())))
						m_baseCheckPos[i] = c->m_transform.m_localTransform.m_position;
				}

				if (auto* bg = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon")))
					m_baseBackgroundPos = bg->m_transform.m_localTransform.m_position;
				m_basePositionsCaptured = true;
			}

			m_backgroundIcon = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));
			if (m_backgroundIcon)
			{
				m_backgroundIcon->m_transform.m_localTransform.m_position = m_baseBackgroundPos + Vector3(0.0f, m_positionOffsetY, 0.0f);
				m_backgroundIcon->m_isDraw = m_isDraw;
			}

			for (int i = 0; i < MAX_ACHIEVEMENT_ROWS; ++i)
			{
				const std::string idxStr = std::to_string(i);
				auto* nameText = GetUI<UIText>(Hash32(("AchieveName_" + idxStr).c_str()));
				auto* boxIcon = GetUI<UIIcon>(Hash32(("AchieveBox_" + idxStr).c_str()));
				auto* checkIcon = GetUI<UIIcon>(Hash32(("AchieveCheck_" + idxStr).c_str()));

				const Vector3 offset(0.0f, m_positionOffsetY, 0.0f);
				if (nameText)  nameText->m_transform.m_localTransform.m_position = m_baseNamePos[i] + offset;
				if (boxIcon)   boxIcon->m_transform.m_localTransform.m_position = m_baseBoxPos[i] + offset;
				if (checkIcon) checkIcon->m_transform.m_localTransform.m_position = m_baseCheckPos[i] + offset;

				if (i < static_cast<int>(achieveList.size()))
				{
					auto* achieve = achieveList[i];
					// JSONのelementsで定義したキー名でUIを取得する
					if (nameText)
					{
						nameText->SetText(achieve->GetDescription());
						nameText->m_isDraw = m_isDraw;
					}
					if (boxIcon)   boxIcon->m_isDraw = m_isDraw;
					if (checkIcon) checkIcon->m_isDraw = m_isDraw && achieve->IsAchieved();

					AchievementUIEntry entry;
					entry.achieve = achieve;
					entry.nameText = nameText;
					entry.boxIcon = boxIcon;
					entry.checkIcon = checkIcon;
					entry.wasAchieved = achieve->IsAchieved();
					m_entries.push_back(entry);
				}
				else
				{
					if (nameText)  nameText->m_isDraw = false;
					if (boxIcon)   boxIcon->m_isDraw = false;
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
					entry.checkIcon->m_isDraw = m_isDraw;
				}

				entry.wasAchieved = isAchieved;
			}

			// Canvasの更新
			MenuBase::Update();
		}


		void InGameAchievementMenu::SetPositionOffsetY(float offsetY)
		{
			if (m_positionOffsetY == offsetY) return;

			m_positionOffsetY = offsetY;

			// 既に生成済みなら即座に位置へ反映する（基準座標は保持されているので多重ずれはしない）
			if (m_basePositionsCaptured)
			{
				InitializeLogic();
			}
		}


		void InGameAchievementMenu::SetDraw(bool isDraw)
		{
			if (m_isDraw == isDraw) return;

			m_isDraw = isDraw;

			if (m_backgroundIcon) m_backgroundIcon->m_isDraw = m_isDraw;

			for (auto& entry : m_entries)
			{
				if (entry.nameText) entry.nameText->m_isDraw = m_isDraw;
				if (entry.boxIcon)  entry.boxIcon->m_isDraw = m_isDraw;
				if (entry.checkIcon)
				{
					// 非表示にする場合は問答無用で隠すが、
					// 再表示する場合は達成済みの行だけチェックアイコンを出す
					entry.checkIcon->m_isDraw = m_isDraw && entry.achieve && entry.achieve->IsAchieved();
				}
			}
		}
	}
}
