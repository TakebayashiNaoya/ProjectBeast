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
#include "Source/Actor/Character/Enemy/EnemyTypes.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Nature/Ocean.h"
#include "Source/Graphics/PBRStatus.h"

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

		/** InGameSceneBase.cpp の SKY_CUBE_SCALE と同じ値 */
		const Vector3 SKY_CUBE_SCALE = Vector3(1000.0f, 800.0f, 1000.0f);

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

		/**
		 * @brief ペンギン（親・子共通、PenguinStateMachine::GetStateNameForLog()）の
		 *        記録stateから対応するアニメーションクリップ番号を返す
		 * @details PenguinIState.cpp の各ステートEnter()が実際に呼ぶPlayAnimation()と対応させている
		 */
		int PenguinAnimIndexForState(const std::string& state)
		{
			using AnimID = actor::EnPenguinAnimationID;
			if (state == "Run")         return static_cast<int>(AnimID::MoveRun);
			if (state == "Sneak")       return static_cast<int>(AnimID::MoveWalk);
			if (state == "Jump")        return static_cast<int>(AnimID::JumpWalking);
			if (state == "SlideStart")  return static_cast<int>(AnimID::SlideStart);
			if (state == "Slide")       return static_cast<int>(AnimID::Sliding);
			if (state == "SlideEnd")    return static_cast<int>(AnimID::StandUp);
			if (state == "Swim")        return static_cast<int>(AnimID::MoveSwim);
			if (state == "InWhirlpool") return static_cast<int>(AnimID::MoveSwim);
			if (state == "Dying")       return static_cast<int>(AnimID::DeathFaceDown);
			if (state == "Dead")        return static_cast<int>(AnimID::DeathFaceDown);
			// "Idle" / "Damaged"（Damagedは実ゲームでも専用アニメが無く直前の姿勢を継続する）/ 未知の値
			return static_cast<int>(AnimID::IdleStanding);
		}

		/**
		 * @brief シロクマ（EnemyStateMachine::GetStateNameForLog()）の記録stateから
		 *        対応するアニメーションクリップ番号を返す
		 * @details EnemyIState.cpp の各ステートEnter()が実際に呼ぶPlayAnimation()と対応させている
		 *          （水中判定は記録していないため、陸上時のアニメーションのみを使う）
		 */
		int BearAnimIndexForState(const std::string& state)
		{
			using AnimID = EnEnemyAnimationType;
			if (state == "Chase")  return static_cast<int>(AnimID::Run);
			if (state == "Attack") return static_cast<int>(AnimID::Attack);
			if (state == "Roar")   return static_cast<int>(AnimID::Buff);
			if (state == "Stun")   return static_cast<int>(AnimID::Stun);
			if (state == "Search") return static_cast<int>(AnimID::BackWalk);
			if (state == "Walk")   return static_cast<int>(AnimID::Walk);
			if (state == "Swim")   return static_cast<int>(AnimID::Swim);
			if (state == "Sleep")  return static_cast<int>(AnimID::Sleep);
			if (state == "Return") return static_cast<int>(AnimID::Walk);
			// "Idle" / "Jump"（専用アニメ無し）/ 未知の値
			return static_cast<int>(AnimID::Idle);
		}
	}


	ReplayScene::ReplayScene()
	{
		ScanSessions();
	}


	ReplayScene::~ReplayScene()
	{
		camera::CameraManager::Get().Unregister(camera::ReplayCamera::ID());
		UnloadBackground();
		g_renderingEngine->UnregisterNatureObject(this);
	}


	bool ReplayScene::Start()
	{
		// リプレイ再生中はこのカメラで撮影した視点を使う
		m_replayCamera = std::make_shared<camera::ReplayCamera>();
		camera::CameraManager::Get().Register(camera::ReplayCamera::ID(), m_replayCamera);
		camera::CameraManager::Get().SwitchCamera(camera::ReplayCamera::ID(), 0.0f);

		// 渦潮（nature::Whirlpool）はGBuffer/ライティング/フォワードパスの後という
		// 専用タイミングでしか正しく描画されないため、Nature Objectとして登録する。
		// 背景（Ocean）読み込み前の暫定登録。LoadBackground()でOceanの後ろに登録し直す
		// （Nature Objectは登録順に描画されるため、渦潮をOceanより後＝海面の上に描く必要がある）
		g_renderingEngine->RegisterNatureObject(this);

		return true;
	}


	void ReplayScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonB))
		{
			m_backToTitle = true;
		}

		if (m_backgroundLoaded)
		{
			actor::StageSystem::GetInstance()->Update();
			nature::Ocean::GetInstance()->Update();
		}

		if (m_isPlaying)
		{
			UpdatePlayback(g_gameTime->GetFrameDeltaTime());
		}

		// カメラ情報が無いログでは、一時停止中も自由に見回せるよう毎フレーム更新する
		if (!m_hasCameraData)
		{
			UpdateFallbackCamera();
		}
	}


	void ReplayScene::PauseUpdate()
	{}


	void ReplayScene::Render(RenderContext& rc)
	{
		DrawUI();

		if (m_backgroundLoaded)
		{
			actor::StageSystem::GetInstance()->Render(rc);
		}

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

		// 渦潮はINatureObject版のRender(rc, view)（下記）で、GBuffer/ライティング/
		// フォワードパスの後という正しいタイミングに描画される
	}


	void ReplayScene::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
	{
		for (size_t i = 0; i < m_whirlpoolModels.size(); i++)
		{
			if (m_whirlpoolSlotActive[i]) m_whirlpoolModels[i]->Render(rc, view);
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

		// カメラ情報が1件でも記録されているログか（古いログには無い場合がある）
		m_hasCameraData = std::any_of(m_ticks.begin(), m_ticks.end(),
			[](const nlohmann::json& t) { return t.contains("camera"); });

		// 再生状態をリセット
		m_isPlaying = false;
		m_playbackTime = 0.0f;
		m_currentTickIndex = 0;
		std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);
		std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);
		std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);

		// このセッションのステージに合わせて背景を読み込む
		const auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
			[&](const SessionEntry& s) { return s.id == sessionId; });
		if (it != m_sessions.end() && !it->stage.empty())
		{
			LoadBackground(it->stage);
		}
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

			// --- 渦潮デバッグ表示（原因切り分け用。表示できたら削除する） ---
			{
				ImGui::Separator();
				const size_t activeCount = std::count(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), true);
				ImGui::Text(u8"[デバッグ] 渦潮プール数:%d アクティブ:%d",
					static_cast<int>(m_whirlpoolModels.size()), static_cast<int>(activeCount));
				for (size_t i = 0; i < m_whirlpoolModels.size(); i++)
				{
					if (!m_whirlpoolSlotActive[i]) continue;
					const auto& t = m_whirlpoolModels[i]->GetTransform();
					ImGui::Text(u8"  #%d id=%d pos=(%.0f,%.0f,%.0f) scale=%.3f state=%d 可視idx=%d",
						static_cast<int>(i), m_whirlpoolSlotIds[i],
						t.m_position.x, t.m_position.y, t.m_position.z,
						t.m_scale.x,
						static_cast<int>(m_whirlpoolModels[i]->GetState()),
						static_cast<int>(m_whirlpoolModels[i]->GetLastVisibleIndexCount()));
				}
			}

			if (!m_hasCameraData)
			{
				ImGui::Separator();
				ImGui::TextDisabled(u8"※このログにはカメラ情報が記録されていません");

				int mode = static_cast<int>(m_noCameraDataMode);
				bool modeChanged = false;
				modeChanged |= ImGui::RadioButton(u8"親ペンギン追従", &mode, static_cast<int>(NoCameraDataMode::FollowParent));
				ImGui::SameLine();
				modeChanged |= ImGui::RadioButton(u8"インスペクター（自由視点）", &mode, static_cast<int>(NoCameraDataMode::Inspector));

				if (m_noCameraDataMode == NoCameraDataMode::FollowParent)
				{
					ImGui::TextDisabled(u8"右スティック:視点回転（追従したまま回せます）");
				}
				else
				{
					ImGui::TextDisabled(u8"左スティック:移動　右スティック:視点回転　RB1+左スティックY:FOV");
				}

				if (modeChanged)
				{
					m_noCameraDataMode = static_cast<NoCameraDataMode>(mode);
					if (m_noCameraDataMode == NoCameraDataMode::Inspector)
					{
						// 切り替え時に現在のカメラ状態から始める（視点が急に飛ばないように）
						m_inspectorCameraData = camera::CameraManager::Get().GetCurrentCameraData();
					}
				}
			}

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

			// UpdateModelOnly() はAI/ステートマシンを動かさないため、PlayAnimation()で
			// 自然に切り替わる機会が無い。記録されたstateが変わったとき（またはInit()直後で
			// クリップ0=CommandShoutが自動再生されて止まっているとき）に明示的に切り替える
			const std::string parentState = p0.value("state", "Idle");
			auto& parentModel = m_parentActor->GetModelRender();
			if (!parentModel.IsPlayingAnimation() || parentState != m_parentLastState)
			{
				parentModel.PlayAnimation(PenguinAnimIndexForState(parentState));
				m_parentLastState = parentState;
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

				// 親ペンギンと同様、記録されたstateの変化に応じて明示的にアニメーションを切り替える
				const std::string bearState = b0.value("state", "Idle");
				auto& bearModel = m_bearActors[slot]->GetModelRender();
				if (!bearModel.IsPlayingAnimation() || bearState != m_bearLastState[slot])
				{
					bearModel.PlayAnimation(BearAnimIndexForState(bearState));
					m_bearLastState[slot] = bearState;
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

				// UpdateAtCountDownTime() は泳ぎ判定以外でアニメーションを切り替えないため、
				// 親ペンギン・シロクマと同様に記録されたstateの変化に応じて明示的に切り替える
				const std::string penguinState = c0.value("state", "Idle");
				auto& penguinModel = m_penguinActors[slot]->GetModelRender();
				if (!penguinModel.IsPlayingAnimation() || penguinState != m_penguinLastState[slot])
				{
					penguinModel.PlayAnimation(PenguinAnimIndexForState(penguinState));
					m_penguinLastState[slot] = penguinState;
				}
			}
		}

		// ------ 渦潮 ------
		if (tick0.contains("whirlpools"))
		{
			std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);

			// Update()（StateMachine）を呼ばないためUV回転は自前で進める
			// （MasterWhirlpoolParameterのuvRotationSpeedのデフォルト値と同じ1.5rad/秒を使う）
			m_whirlpoolUvRotation += deltaTime * m_playbackSpeed * 1.5f;

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
				m_whirlpoolModels[slot]->SetUvRotation(m_whirlpoolUvRotation);
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


	void ReplayScene::UpdateFallbackCamera()
	{
		if (!m_replayCamera) return;

		if (m_noCameraDataMode == NoCameraDataMode::FollowParent)
		{
			if (!m_parentActor) return;

			// 右スティックでターゲット（親ペンギン）中心にオフセットを回転させる
			const float rotX = g_pad[0]->GetRStickXF() * 0.05f;
			const float rotY = g_pad[0]->GetRStickYF() * 0.05f;
			if (rotX != 0.0f || rotY != 0.0f)
			{
				Quaternion yRotation;
				yRotation.SetRotationY(rotX);
				yRotation.Apply(m_followCameraOffset);

				Vector3 rightDir;
				rightDir.Cross(Vector3::Up, m_followCameraOffset);
				rightDir.Normalize();

				Quaternion xzRotation;
				xzRotation.SetRotation(rightDir, rotY);
				xzRotation.Apply(m_followCameraOffset);

				// 上下角度をクランプ（真上・真下まで回り込みすぎないように）
				const float length = m_followCameraOffset.Length();
				const float maxAngle = Math::DegToRad(80.0f);
				const float minAngle = Math::DegToRad(-30.0f);
				const float maxY = sinf(maxAngle) * length;
				const float minY = sinf(minAngle) * length;
				m_followCameraOffset.y = std::clamp(m_followCameraOffset.y, minY, maxY);

				const float xzLenSq = length * length - m_followCameraOffset.y * m_followCameraOffset.y;
				const float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;
				Vector3 xzDir(m_followCameraOffset.x, 0.0f, m_followCameraOffset.z);
				if (xzDir.LengthSq() > FLT_EPSILON)
				{
					xzDir.Normalize();
				}
				else
				{
					xzDir.Set(0.0f, 0.0f, -1.0f);
				}
				m_followCameraOffset.x = xzDir.x * xzLen;
				m_followCameraOffset.z = xzDir.z * xzLen;
			}

			// ターゲット自体は毎フレーム親ペンギンの現在座標に追従する
			const Vector3& targetPos = m_parentActor->GetTransform().m_position;
			camera::CameraData data;
			data.target = targetPos + Vector3(0.0f, 60.0f, 0.0f);
			data.position = data.target + m_followCameraOffset;
			data.up = Vector3::Up;

			m_replayCamera->SetState(data);
		}
		else
		{
			UpdateInspectorCamera();
		}
	}


	void ReplayScene::UpdateInspectorCamera()
	{
		if (!m_replayCamera) return;

		// fov調整
		if (g_pad[0]->IsPress(enButtonRB1))
		{
			const float value = g_pad[0]->GetLStickYF();
			m_inspectorCameraData.fov += value * 0.05f;
			m_replayCamera->SetState(m_inspectorCameraData);
			return;
		}

		// 左スティックで平行移動
		{
			Vector3 inputDirection;
			inputDirection.x = g_pad[0]->GetLStickXF();
			inputDirection.z = g_pad[0]->GetLStickYF();

			Vector3 forward = CameraSystem::Get().GetMainCamera().GetForward();
			Vector3 right = CameraSystem::Get().GetMainCamera().GetRight();
			forward.y = 0.0f;
			right.y = 0.0f;

			right *= inputDirection.x;
			forward *= inputDirection.z;

			Vector3 direction = right + forward;
			direction.Normalize();
			direction.Scale(10.0f);

			m_inspectorCameraData.position += direction;
			m_inspectorCameraData.target += direction;
		}

		// 右スティックでターゲット中心に回転
		{
			const float rotX = g_pad[0]->GetRStickXF() * 0.05f;
			const float rotY = g_pad[0]->GetRStickYF() * 0.05f;

			Quaternion yRotation;
			yRotation.SetRotationY(rotX);
			Vector3 toVector = m_inspectorCameraData.position - m_inspectorCameraData.target;
			yRotation.Apply(toVector);

			Vector3 rightDir;
			rightDir.Cross(Vector3::Up, toVector);
			rightDir.Normalize();

			Quaternion xzRotation;
			xzRotation.SetRotation(rightDir, rotY);
			xzRotation.Apply(toVector);

			const float length = toVector.Length();
			const float maxAngle = Math::DegToRad(85.0f);
			const float minAngle = Math::DegToRad(-85.0f);
			const float maxY = sinf(maxAngle) * length;
			const float minY = sinf(minAngle) * length;

			toVector.y = std::clamp(toVector.y, minY, maxY);

			const float xzLenSq = length * length - toVector.y * toVector.y;
			const float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;

			Vector3 xzDir;
			xzDir.Set(toVector.x, 0.0f, toVector.z);
			if (xzDir.LengthSq() > FLT_EPSILON)
			{
				xzDir.Normalize();
			}
			else
			{
				xzDir.Set(0.0f, 0.0f, -1.0f);
			}

			toVector.x = xzDir.x * xzLen;
			toVector.z = xzDir.z * xzLen;

			m_inspectorCameraData.position = m_inspectorCameraData.target + toVector;
		}

		m_replayCamera->SetState(m_inspectorCameraData);
	}


	void ReplayScene::LoadBackground(const std::string& stageName)
	{
		if (m_backgroundLoaded && m_loadedStageName == stageName) return;

		UnloadBackground();

		// InGameSceneBase の各ステージシーン（例: NormalInGameScene）が返すパスと同じ命名規則
		const std::string stageJsonPath  = "Assets/parameter/stage/StageObject_" + stageName + ".json";
		const std::string terrainJsonPath = "Assets/parameter/stage/TerrainConfig_" + stageName + ".json";
		const std::string oceanJsonPath  = "Assets/parameter/nature/oceanParameter_" + stageName + ".json";
		const std::string oceanBinPath   = "Assets/parameter/nature/oceanParameter_" + stageName + ".bin";

		// StageSystem・Oceanが内部でPBRパラメータ・ParameterManagerを参照するため生成しておく
		graphics::PBRStatus::CreateInstance();

		actor::StageSystem::CreateInstance();
		actor::StageSystem::GetInstance()->LoadStageObjectsFromJson(stageJsonPath);
		actor::StageSystem::GetInstance()->InitTerrainFromJson(terrainJsonPath);

		m_skyCube = NewGO<SkyCube>(0);
		m_skyCube->SetType(enSkyCubeType_Clear);
		m_skyCube->SetScale(SKY_CUBE_SCALE);
		m_skyCube->SetLuminance(0.8f);

		nature::Ocean::CreateInstance();
		nature::Ocean::GetInstance()->Start(oceanBinPath.c_str(), oceanJsonPath.c_str());

		// Nature Objectは登録順に描画される。渦潮用のReplayScene自身の登録がOceanより先に
		// 済んでいると、海面が渦潮の上から描かれて隠れてしまうため、Oceanの後ろに登録し直す
		g_renderingEngine->UnregisterNatureObject(this);
		g_renderingEngine->RegisterNatureObject(this);

		m_backgroundLoaded = true;
		m_loadedStageName = stageName;
	}


	void ReplayScene::UnloadBackground()
	{
		if (!m_backgroundLoaded) return;

		DeleteGO(m_skyCube);
		m_skyCube = nullptr;

		nature::Ocean::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		graphics::PBRStatus::DestroyInstance();

		m_backgroundLoaded = false;
		m_loadedStageName.clear();
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
		m_bearLastState.push_back("");

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
		m_penguinLastState.push_back("");

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
