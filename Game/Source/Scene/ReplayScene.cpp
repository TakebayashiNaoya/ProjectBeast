/**
 * @file ReplayScene.cpp
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#include "stdafx.h"
#include "ReplayScene.h"

#include "TitleScene.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"

#include <algorithm>
#include <filesystem>
#include <fstream>


namespace app
{
	namespace
	{
		/**
		 * @brief 1倍速再生時に1秒あたり何tick進めるか
		 * @details 記録データの "t" は TimeManager の残り時間（カウントダウン）を
		 *          そのまま使っており単調増加しないため、再生の時間軸には使えない。
		 *          "frame"（tick連番、確実に1ずつ増える）を直接タイムライン単位として使い、
		 *          "何秒間隔で記録されたログか"（InGameSceneBase.cpp の LOG_TICK_INTERVAL）には
		 *          依存しない。LOG_TICK_INTERVAL は過去に値を変更した経緯があり、
		 *          ログごとに実際の記録間隔が異なりうるため。
		 */
		constexpr float TICKS_PER_SECOND = 10.0f;

		/**
		 * @brief シロクマのアイドルアニメーションのクリップ番号
		 * @details Enemy.cpp の ANIMATION_DATA[] の並び順に対応する固定インデックス
		 *          （0 = "Assets/animData/bear/idle.tka"）。Enemyはenum非公開のため直値で持つ。
		 */
		constexpr int BEAR_IDLE_ANIM_INDEX = 0;

		/** json配列 [x,y,z] を Vector3 に変換する */
		Vector3 JsonToV3(const nlohmann::json& j)
		{
			return Vector3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
		}

		/** GameLogManager が記録したタイプ名文字列を EnChildPenguinType に変換する */
		actor::EnChildPenguinType ParseChildPenguinType(const std::string& typeStr)
		{
			if (typeStr == "Serious") return actor::EnChildPenguinType::Serious;
			if (typeStr == "Clingy")  return actor::EnChildPenguinType::Clingy;
			if (typeStr == "Naughty") return actor::EnChildPenguinType::Naughty;
			if (typeStr == "Clumsy")  return actor::EnChildPenguinType::Clumsy;
			if (typeStr == "Caring")  return actor::EnChildPenguinType::Caring;
			return actor::EnChildPenguinType::Serious;
		}
	}


	ReplayScene::ReplayScene()
	{
		ScanSessions();
	}


	ReplayScene::~ReplayScene()
	{
		camera::CameraManager::Get().Unregister(camera::ReplayCamera::ID());
	}


	bool ReplayScene::Start()
	{
		// リプレイ再生中はこのカメラで撮影した視点を使う
		m_replayCamera = std::make_shared<camera::ReplayCamera>();
		camera::CameraManager::Get().Register(camera::ReplayCamera::ID(), m_replayCamera);
		camera::CameraManager::Get().SwitchCamera(camera::ReplayCamera::ID(), 0.0f);

		return true;
	}


	void ReplayScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonB))
		{
			m_backToTitle = true;
		}

		if (m_isPlaying)
		{
			UpdatePlayback(g_gameTime->GetFrameDeltaTime());
		}
	}


	void ReplayScene::PauseUpdate()
	{}


	void ReplayScene::Render(RenderContext& rc)
	{
		DrawUI();

		if (m_parentActor)
		{
			m_parentActor->RenderWrapper(rc);
		}

		for (size_t i = 0; i < m_bearActors.size(); i++)
		{
			if (m_bearSlotActive[i]) m_bearActors[i]->RenderWrapper(rc);
		}

		for (size_t i = 0; i < m_penguinActors.size(); i++)
		{
			if (m_penguinSlotActive[i]) m_penguinActors[i]->RenderWrapper(rc);
		}

		for (size_t i = 0; i < m_whirlpoolModels.size(); i++)
		{
			if (m_whirlpoolSlotActive[i]) m_whirlpoolModels[i]->RenderWrapper(rc);
		}
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

			// spawn/despawnイベント行は座標情報を持たず再生に使えないため除外する。
			// "type":"tick" の行だけを残すことで、"frame" が確実に連番になる
			if (tick.value("type", "") != "tick") continue;

			m_ticks.push_back(std::move(tick));
		}

		m_loadedSessionId = sessionId;

		// 再生状態をリセット
		m_isPlaying = false;
		m_playbackTime = 0.0f;
		m_currentTickIndex = 0;
		std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);
		std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);
		std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);
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
			ImGui::Separator();

			if (!m_isPlaying)
			{
				if (ImGui::Button(u8"再生"))
				{
					StartPlayback();
				}
			}
			else
			{
				if (ImGui::Button(u8"停止"))
				{
					m_isPlaying = false;
				}
				ImGui::SameLine();
				ImGui::Text(u8"再生時間: %.1f秒相当", m_playbackTime / TICKS_PER_SECOND);
			}

			ImGui::SliderFloat(u8"再生速度", &m_playbackSpeed, 0.1f, 4.0f);

			ImGui::Separator();
			if (ImGui::Button(u8"別のログを選び直す"))
			{
				m_loadedSessionId.clear();
				m_ticks.clear();
				m_isPlaying = false;
				std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);
				std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);
				std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);
			}
		}

		ImGui::Separator();
		ImGui::Text(u8"Bボタンでタイトルへ戻る");

		ImGui::End();
	}


	void ReplayScene::StartPlayback()
	{
		if (m_ticks.empty()) return;

		m_isPlaying = true;
		m_playbackTime = static_cast<float>(m_ticks.front().value("frame", 0));
		m_currentTickIndex = 0;
	}


	void ReplayScene::UpdatePlayback(float deltaTime)
	{
		if (m_ticks.empty())
		{
			m_isPlaying = false;
			return;
		}

		m_playbackTime += deltaTime * m_playbackSpeed * TICKS_PER_SECOND;

		// 現在の再生時間を含むtickペアまでインデックスを進める
		while (m_currentTickIndex + 1 < m_ticks.size() &&
			m_ticks[m_currentTickIndex + 1].value("frame", 0) <= m_playbackTime)
		{
			m_currentTickIndex++;
		}

		// 最終tickに到達したら再生終了（最後の姿勢のまま止める）
		if (m_currentTickIndex + 1 >= m_ticks.size())
		{
			m_isPlaying = false;
		}

		const nlohmann::json& tick0 = m_ticks[m_currentTickIndex];
		const nlohmann::json& tick1 = m_ticks[min(m_currentTickIndex + 1, m_ticks.size() - 1)];

		const float t0 = static_cast<float>(tick0.value("frame", 0));
		const float t1 = static_cast<float>(tick1.value("frame", 0));
		float alpha = (t1 > t0) ? (m_playbackTime - t0) / (t1 - t0) : 0.0f;
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		// ------ 親ペンギン ------
		if (tick0.contains("parent"))
		{
			const auto& p0 = tick0["parent"];
			const auto& p1 = tick1.contains("parent") ? tick1["parent"] : p0;

			Vector3 pos;
			pos.Lerp(alpha, JsonToV3(p0["pos"]), JsonToV3(p1["pos"]));

			const float rotY0 = p0.value("rot_y", 0.0f);
			const float rotY1 = p1.value("rot_y", rotY0);

			if (!m_parentActor)
			{
				m_parentActor = std::make_unique<actor::DaddyPenguin>();
				m_parentActor->StartWrapper();
			}

			Quaternion rot;
			rot.SetRotationDegY(rotY0 + (rotY1 - rotY0) * alpha);
			m_parentActor->SetPosition(pos);
			m_parentActor->SetRotation(rot);
			m_parentActor->UpdateModelOnly();

			// UpdateModelOnly() はAI/ステートマシンを動かさないため、
			// PlayAnimation()で自然に切り替わる機会が無い。Init()直後は
			// クリップ0（CommandShout、ループ無し）が自動再生されて即停止してしまうため、
			// ロード完了を待ってから明示的にアイドルへ切り替える
			auto& parentModel = m_parentActor->GetModelRender();
			if (!parentModel.IsPlayingAnimation())
			{
				parentModel.PlayAnimation(static_cast<int>(actor::EnPenguinAnimationID::IdleStanding));
			}
		}

		// ------ シロクマ ------
		if (tick0.contains("bears"))
		{
			std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);

			const auto& bears0 = tick0["bears"];
			const auto& bears1 = tick1.contains("bears") ? tick1["bears"] : bears0;

			for (const auto& b0 : bears0)
			{
				const int id = b0.value("id", -1);

				// tick1側で同じidを探す。見つからなければ補間せずtick0の値をそのまま使う
				const nlohmann::json* b1 = nullptr;
				for (const auto& cand : bears1)
				{
					if (cand.value("id", -2) == id) { b1 = &cand; break; }
				}

				const size_t slot = AcquireBearSlot(id);
				m_bearSlotActive[slot] = true;

				Vector3 pos0 = JsonToV3(b0["pos"]);
				Vector3 pos1 = b1 ? JsonToV3((*b1)["pos"]) : pos0;
				Vector3 pos;
				pos.Lerp(alpha, pos0, pos1);

				const float rotY0 = b0.value("rot_y", 0.0f);
				const float rotY1 = b1 ? b1->value("rot_y", rotY0) : rotY0;

				Quaternion rot;
				rot.SetRotationDegY(rotY0 + (rotY1 - rotY0) * alpha);
				m_bearActors[slot]->SetPosition(pos);
				m_bearActors[slot]->SetRotation(rot);
				m_bearActors[slot]->UpdateModelOnly();

				// 親ペンギンと同様、UpdateModelOnly()だけではPlayAnimation()が呼ばれる機会が無いため、
				// ロード完了後に明示的にアイドルへ切り替える
				auto& bearModel = m_bearActors[slot]->GetModelRender();
				if (!bearModel.IsPlayingAnimation())
				{
					bearModel.PlayAnimation(BEAR_IDLE_ANIM_INDEX);
				}
			}
		}

		// ------ 子ペンギン ------
		if (tick0.contains("penguins"))
		{
			std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);

			const auto& penguins0 = tick0["penguins"];
			const auto& penguins1 = tick1.contains("penguins") ? tick1["penguins"] : penguins0;

			for (const auto& c0 : penguins0)
			{
				const int id = c0.value("id", -1);

				const nlohmann::json* c1 = nullptr;
				for (const auto& cand : penguins1)
				{
					if (cand.value("id", -2) == id) { c1 = &cand; break; }
				}

				const size_t slot = AcquirePenguinSlot(id, c0.value("type", "Serious"));
				m_penguinSlotActive[slot] = true;

				Vector3 pos0 = JsonToV3(c0["pos"]);
				Vector3 pos1 = c1 ? JsonToV3((*c1)["pos"]) : pos0;
				Vector3 pos;
				pos.Lerp(alpha, pos0, pos1);

				const float rotY0 = c0.value("rot_y", 0.0f);
				const float rotY1 = c1 ? c1->value("rot_y", rotY0) : rotY0;

				Quaternion rot;
				rot.SetRotationDegY(rotY0 + (rotY1 - rotY0) * alpha);
				m_penguinActors[slot]->SetPosition(pos);
				m_penguinActors[slot]->SetRotation(rot);
				m_penguinActors[slot]->UpdateAtCountDownTime();
			}
		}

		// ------ 渦潮 ------
		if (tick0.contains("whirlpools"))
		{
			std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);

			const auto& wp0 = tick0["whirlpools"];
			const auto& wp1 = tick1.contains("whirlpools") ? tick1["whirlpools"] : wp0;

			for (const auto& w0 : wp0)
			{
				const int id = w0.value("id", -1);

				const nlohmann::json* w1 = nullptr;
				for (const auto& cand : wp1)
				{
					if (cand.value("id", -2) == id) { w1 = &cand; break; }
				}

				const size_t slot = AcquireWhirlpoolSlot(m_whirlpoolSlotIds, m_whirlpoolSlotActive, id);
				m_whirlpoolSlotActive[slot] = true;

				Vector3 pos0 = JsonToV3(w0["pos"]);
				Vector3 pos1 = w1 ? JsonToV3((*w1)["pos"]) : pos0;
				Vector3 pos;
				pos.Lerp(alpha, pos0, pos1);

				const float scale0 = w0.value("scale_xz", 1.0f);
				const float scale1 = w1 ? w1->value("scale_xz", scale0) : scale0;

				m_whirlpoolModels[slot]->SetPosition(pos);
				m_whirlpoolModels[slot]->SetScaleXZ(scale0 + (scale1 - scale0) * alpha);
			}
		}

		// ------ カメラ ------
		if (tick0.contains("camera") && m_replayCamera)
		{
			const auto& cam0 = tick0["camera"];
			const auto& cam1 = tick1.contains("camera") ? tick1["camera"] : cam0;

			camera::CameraData data;
			data.position.Lerp(alpha, JsonToV3(cam0["pos"]), JsonToV3(cam1["pos"]));
			data.target.Lerp(alpha, JsonToV3(cam0["target"]), JsonToV3(cam1["target"]));
			data.up = Vector3::Up;

			const float fov0 = cam0.value("fov", data.fov);
			const float fov1 = cam1.value("fov", fov0);
			data.fov = fov0 + (fov1 - fov0) * alpha;

			m_replayCamera->SetState(data);
		}
	}


	size_t ReplayScene::AcquireBearSlot(int id)
	{
		for (size_t i = 0; i < m_bearSlotIds.size(); i++)
		{
			if (m_bearSlotIds[i] == id) return i;
		}

		auto enemy = std::make_unique<actor::Enemy>();
		enemy->StartWrapper();
		m_bearActors.push_back(std::move(enemy));
		m_bearSlotIds.push_back(id);
		m_bearSlotActive.push_back(false);

		return m_bearSlotIds.size() - 1;
	}


	size_t ReplayScene::AcquirePenguinSlot(int id, const std::string& typeStr)
	{
		for (size_t i = 0; i < m_penguinSlotIds.size(); i++)
		{
			if (m_penguinSlotIds[i] == id) return i;
		}

		auto penguin = std::make_unique<actor::ChildPenguin>();
		penguin->SetChildPenguinType(ParseChildPenguinType(typeStr));
		penguin->StartWrapper();
		m_penguinActors.push_back(std::move(penguin));
		m_penguinSlotIds.push_back(id);
		m_penguinSlotActive.push_back(false);

		return m_penguinSlotIds.size() - 1;
	}


	size_t ReplayScene::AcquireWhirlpoolSlot(std::vector<int>& slotIds, std::vector<bool>& slotActive, int id)
	{
		for (size_t i = 0; i < slotIds.size(); i++)
		{
			if (slotIds[i] == id) return i;
		}

		auto whirlpool = std::make_unique<nature::Whirlpool>();
		whirlpool->StartWrapper();
		// リプレイではUpdate()（状態遷移）を呼ばないため、Start()が自動再生したエフェクトを
		// ここで明示的に止める。止めないと二度と止まらずセッション中の全渦潮分が蓄積してしまう
		whirlpool->StopEffect();
		m_whirlpoolModels.push_back(std::move(whirlpool));
		slotIds.push_back(id);
		slotActive.push_back(false);

		return slotIds.size() - 1;
	}
}
