/**
 * @file ReplayScene.cpp
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#include "stdafx.h"
#include "ReplayScene.h"

#include "TitleScene.h"

#include <algorithm>
#include <filesystem>
#include <fstream>


namespace app
{
	ReplayScene::ReplayScene()
	{
		ScanSessions();
	}


	ReplayScene::~ReplayScene()
	{}


	bool ReplayScene::Start()
	{
		return true;
	}


	void ReplayScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonB))
		{
			m_backToTitle = true;
		}
	}


	void ReplayScene::PauseUpdate()
	{}


	void ReplayScene::Render(RenderContext& rc)
	{
		DrawUI();
	}


	bool ReplayScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_backToTitle)
		{
			id = TitleScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}


	void ReplayScene::ScanSessions()
	{
		m_sessions.clear();

		const std::filesystem::path logsDir = "Logs";
		if (!std::filesystem::exists(logsDir)) return;

		for (const auto& entry : std::filesystem::directory_iterator(logsDir))
		{
			if (!entry.is_directory()) continue;

			SessionEntry session;
			session.id = entry.path().filename().string();

			std::ifstream ifs(entry.path() / "session.json");
			if (ifs)
			{
				const nlohmann::json j = nlohmann::json::parse(ifs, nullptr, false);
				if (!j.is_discarded())
				{
					session.stage       = j.value("stage", "");
					session.durationSec = j.value("duration_sec", 0.0f);

					if (j.contains("result"))
					{
						session.rescued = j["result"].value("rescued", 0);
						session.score   = j["result"].value("score", 0.0f);
					}
				}
			}

			m_sessions.push_back(std::move(session));
		}

		// session_id は "%Y-%m-%d_%H-%M-%S" 形式なので文字列の降順ソート＝新しい順になる
		std::sort(m_sessions.begin(), m_sessions.end(),
			[](const SessionEntry& a, const SessionEntry& b) { return a.id > b.id; });
	}


	void ReplayScene::LoadSession(const std::string& sessionId)
	{
		m_ticks.clear();

		std::ifstream ifs("Logs/" + sessionId + "/ticks.jsonl");
		if (!ifs) return;

		std::string line;
		while (std::getline(ifs, line))
		{
			if (line.empty()) continue;

			nlohmann::json tick = nlohmann::json::parse(line, nullptr, false);
			if (tick.is_discarded()) continue;

			m_ticks.push_back(std::move(tick));
		}

		m_loadedSessionId = sessionId;
	}


	void ReplayScene::DrawUI()
	{
		ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(500.0f, 400.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin(u8"リプレイ");

		if (m_loadedSessionId.empty())
		{
			ImGui::Text(u8"再生するログを選択してください");
			ImGui::Separator();

			if (m_sessions.empty())
			{
				ImGui::TextDisabled(u8"（Logs/ にセッションが見つかりません）");
			}

			for (const auto& session : m_sessions)
			{
				char label[256];
				sprintf_s(label, u8"%s  [%s]  %.0f秒  救出%d  score:%.0f",
					session.id.c_str(), session.stage.c_str(),
					session.durationSec, session.rescued, session.score);

				if (ImGui::Selectable(label))
				{
					LoadSession(session.id);
				}
			}
		}
		else
		{
			ImGui::Text(u8"読み込み済み: %s", m_loadedSessionId.c_str());
			ImGui::Text(u8"tick数: %d", static_cast<int>(m_ticks.size()));
			ImGui::Text(u8"（再生ロジックは未実装）");

			if (ImGui::Button(u8"別のログを選び直す"))
			{
				m_loadedSessionId.clear();
				m_ticks.clear();
			}
		}

		ImGui::Separator();
		ImGui::Text(u8"Bボタンでタイトルへ戻る");

		ImGui::End();
	}
}
