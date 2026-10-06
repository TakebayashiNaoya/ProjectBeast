/**
 * @file StageInfoPanel.cpp
 * @brief ステージ選択画面の情報パネル（制限時間・クマ数・渦潮数・記録）
 */
#include "stdafx.h"
#include "StageInfoPanel.h"

#include "Source/Manager/ScoreManager.h"
// 制限時間・ステージ名・配置JSONパスの一次資料（STAGE_INFO_TABLE）を共有するため
#include "Source/Scene/InGameSceneBase.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr uint32_t TIME_TEXT_KEY = Hash32("StageInfoTimeText");
			constexpr uint32_t BEAR_TEXT_KEY = Hash32("StageInfoBearText");
			constexpr uint32_t WHIRL_TEXT_KEY = Hash32("StageInfoWhirlText");
			constexpr uint32_t RECORD_TEXT_KEY = Hash32("StageInfoRecordText");

			/** パネルを構成する全パーツ（パーツを足すときはここに追記する） */
			constexpr uint32_t PART_KEYS[] =
			{
				Hash32("StageInfoPanel"),
				Hash32("StageInfoTimeIcon"),
				TIME_TEXT_KEY,
				Hash32("StageInfoBearIcon"),
				BEAR_TEXT_KEY,
				Hash32("StageInfoWhirlIcon"),
				WHIRL_TEXT_KEY,
				RECORD_TEXT_KEY,
			};
		}


		StageInfoPanel::StageInfoPanel()
			: m_timeText(nullptr)
			, m_bearText(nullptr)
			, m_whirlText(nullptr)
			, m_recordText(nullptr)
			, m_bearCounts()
			, m_whirlCounts()
			, m_isCountsLoaded(false)
		{}


		void StageInfoPanel::Initialize(MenuBase& menu)
		{
			m_parts.clear();
			m_baseAlphas.clear();
			for (const uint32_t key : PART_KEYS)
			{
				auto* part = menu.GetUI<UIBase>(key);
				K2_ASSERT(part, "情報パネルのパーツを取得できていません。");
				m_parts.push_back(part);
				m_baseAlphas.push_back(part->m_color.w);
			}

			m_timeText = menu.GetUI<UIText>(TIME_TEXT_KEY);
			m_bearText = menu.GetUI<UIText>(BEAR_TEXT_KEY);
			m_whirlText = menu.GetUI<UIText>(WHIRL_TEXT_KEY);
			m_recordText = menu.GetUI<UIText>(RECORD_TEXT_KEY);
			K2_ASSERT(m_timeText && m_bearText && m_whirlText && m_recordText,
				"情報パネルのテキストを取得できていません。");
		}


		void StageInfoPanel::SetStage(const int stageIndex)
		{
			if (stageIndex < 0 || stageIndex >= STAGE_NUM) return;

			LoadCountsIfNeeded();

			/** 制限時間もステージ名も、インゲームシーンと同じ STAGE_INFO_TABLE から引く */
			const StageInfo& info = STAGE_INFO_TABLE[stageIndex];
			const int timeSeconds = static_cast<int>(info.timeLimit);

			char buf[48];
			sprintf_s(buf, "%d:%02d", timeSeconds / 60, timeSeconds % 60);
			m_timeText->SetText(buf);

			sprintf_s(buf, "x%d", m_bearCounts[stageIndex]);
			m_bearText->SetText(buf);

			sprintf_s(buf, "x%d", m_whirlCounts[stageIndex]);
			m_whirlText->SetText(buf);

			const int highScore = ScoreManager::GetHighScore(info.name);
			if (highScore > 0)
			{
				sprintf_s(buf, "きろく %d", highScore);
				m_recordText->SetText(buf);
			}
			else
			{
				m_recordText->SetText("きろく ---");
			}
		}


		void StageInfoPanel::SetDraw(const bool isDraw)
		{
			for (auto* part : m_parts)
			{
				part->SetIsDraw(isDraw);
			}
		}


		void StageInfoPanel::SetAlpha(const float rate)
		{
			for (size_t i = 0; i < m_parts.size(); ++i)
			{
				m_parts[i]->m_color.w = m_baseAlphas[i] * rate;
			}
		}


		void StageInfoPanel::LoadCountsIfNeeded()
		{
			static_assert(STAGE_NUM == STAGE_INFO_COUNT,
				"情報パネルの枠数と STAGE_INFO_TABLE の件数を揃えること");

			if (m_isCountsLoaded) return;
			m_isCountsLoaded = true;

			/** 配置JSONの実データから数を数える。
			 *  パスはインゲームシーンと共有の STAGE_INFO_TABLE から引くので、
			 *  ステージを再生成・改名しても表示が自動で追従する */
			for (int i = 0; i < STAGE_NUM; ++i)
			{
				const StageInfo& info = STAGE_INFO_TABLE[i];

				nlohmann::json json;
				if (util::JsonConverter::IsLoadJsonFile(json, info.enemyLayoutJsonPath)
					&& json.contains("enemies"))
				{
					m_bearCounts[i] = static_cast<int>(json["enemies"].size());
				}
				if (util::JsonConverter::IsLoadJsonFile(json, info.whirlpoolPositionsJsonPath)
					&& json.contains("whirlpoolPositions"))
				{
					m_whirlCounts[i] = static_cast<int>(json["whirlpoolPositions"].size());
				}
			}
		}
	}
}
