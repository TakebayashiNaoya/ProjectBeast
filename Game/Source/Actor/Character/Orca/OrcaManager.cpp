/**
 * @file OrcaManager.cpp
 * @brief シャチの管理
 */
#include "stdafx.h"
#include "OrcaManager.h"
#include "Orca.h"
#include "OrcaController.h"
#include "OrcaStateMachine.h"
#include "OrcaStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Actor/Stage/TerrainObject.h"
#include "Source/Core/DebugWindow.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace actor
	{
		OrcaManager* OrcaManager::m_instance = nullptr;


		namespace
		{
			/** デバッグ欄の見出し */
			constexpr const char* DEBUG_LABEL = u8"シャチ";
			/** デバッグ欄に描く地図の1辺の最大画素数 */
			constexpr int DEBUG_MAP_MAX_PIXELS = 240;

			/** デバッグ地図の色 */
			constexpr uint32_t MAP_COLOR_NAVIGABLE = IM_COL32(40, 110, 200, 255);	// 泳げる
			constexpr uint32_t MAP_COLOR_SEA = IM_COL32(20, 60, 90, 255);			// 海だが岸に近すぎる
			constexpr uint32_t MAP_COLOR_LAND = IM_COL32(150, 150, 150, 255);		// 浅い・陸・外周の外
			constexpr uint32_t MAP_COLOR_PATROL = IM_COL32(255, 220, 0, 255);
			constexpr uint32_t MAP_COLOR_ORCA = IM_COL32(255, 60, 60, 255);
			constexpr uint32_t MAP_COLOR_DADDY = IM_COL32(255, 255, 255, 255);

			/** デバッグ表示を有効にするビルドか */
#if defined(_DEBUG) || defined(K2_DEBUG)
			constexpr bool IS_DEBUG_BUILD = true;
#else
			constexpr bool IS_DEBUG_BUILD = false;
#endif
		}


		OrcaManager::OrcaManager()
		{}


		OrcaManager::~OrcaManager()
		{
			ClearOrcas();
			if (m_isDebugRegistered)
			{
				DebugWindow::Get().Unregister(DEBUG_LABEL);
				m_isDebugRegistered = false;
			}
		}


		void OrcaManager::ClearOrcas()
		{
			for (auto& data : m_orcaList)
			{
				delete data.controller;
				delete data.orca;
			}
			m_orcaList.clear();
		}


		void OrcaManager::LoadOrcas(const nlohmann::json& json)
		{
			// --- 泳げる場所の地図 ---
			OrcaSeaGrid::Settings settings;
			if (json.contains("sea"))
			{
				const auto& sea = json["sea"];
				settings.cellSize = util::JsonConverter::ToFloat(sea, "cellSize", settings.cellSize);
				settings.seaFloorMaxY = util::JsonConverter::ToFloat(sea, "seaFloorMaxY", settings.seaFloorMaxY);
				settings.bodyClearance = util::JsonConverter::ToFloat(sea, "bodyClearance", settings.bodyClearance);
				settings.boundaryRadius = util::JsonConverter::ToFloat(sea, "boundaryRadius", settings.boundaryRadius);
			}

			const TerrainObject* terrain = StageSystem::GetInstance() ? StageSystem::GetInstance()->GetTerrain() : nullptr;
			if (terrain == nullptr || !json.contains("orcas") || json["orcas"].empty())
			{
				return;
			}

			LARGE_INTEGER begin, end, frequency;
			QueryPerformanceCounter(&begin);
			m_seaGrid.Build(settings, [terrain](float x, float z)
			{
				return terrain->GetHeightAt(Vector3(x, 0.0f, z));
			});
			QueryPerformanceCounter(&end);
			QueryPerformanceFrequency(&frequency);
			m_buildMilliseconds = 1000.0 * static_cast<double>(end.QuadPart - begin.QuadPart) / static_cast<double>(frequency.QuadPart);

			if (IS_DEBUG_BUILD)
			{
				// 合成地形で地図の約束（海から出ない等）を確かめる
				const bool passed = OrcaSeaGrid::SelfTest(m_selfTestMessage);
				K2_ASSERT(passed, "OrcaSeaGrid のセルフテストに失敗しました。デバッグ欄「シャチ」を確認してください");
			}

			// --- シャチの生成 ---
			for (const auto& orcaJson : json["orcas"])
			{
				// 出現地点は航行可能な点に寄せる
				const Vector3 requestedSpawn = util::JsonConverter::ToVector3(orcaJson, "spawnPosition");
				Vector3 spawn;
				if (!m_seaGrid.FindNearestNavigable(requestedSpawn, -1, spawn))
				{
					K2_ASSERT(false, "シャチが泳げる場所がありません。OrcaLayout の sea の値を確認してください");
					continue;
				}
				if (!m_seaGrid.IsNavigable(requestedSpawn)) m_snappedPointCount++;
				const int component = m_seaGrid.GetComponent(spawn);

				OrcaData data;
				data.controller = new OrcaController();

				// 巡回点も、出現地点と同じ水域の航行可能な点に寄せる
				if (orcaJson.contains("patrolPoints"))
				{
					for (const auto& pointJson : orcaJson["patrolPoints"])
					{
						const Vector3 requested = util::JsonConverter::ToVector3(pointJson);
						Vector3 point;
						if (!m_seaGrid.FindNearestNavigable(requested, component, point)) continue;
						if (m_seaGrid.GetComponent(requested) != component) m_snappedPointCount++;
						data.controller->AddPatrolPoint(point);
					}
				}

				data.orca = new Orca();
				data.orca->SetPosition(spawn);
				data.orca->SetSeaGrid(&m_seaGrid);
				data.orca->SetLogId(static_cast<int>(m_orcaList.size()));

				// 出現地点と最初の巡回点が同じことが多いので、2つ目の巡回点の方を向いて出現する（Start で姿勢を決めるより前に設定する）
				const auto& patrol = data.controller->GetPatrolPoints();
				if (patrol.size() >= 2)
				{
					data.orca->GetOrcaStateMachine()->SetFacing(patrol[1] - spawn);
				}
				data.orca->StartWrapper();
				data.controller->SetTarget(data.orca);

				m_orcaList.push_back(data);
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->RecordSpawn("orca", data.orca->GetLogId());
				}
			}
			// 出現地点・巡回点を寄せた件数はデバッグ欄「シャチ」に出す（止めるほどではないので assert にはしない）

			if (IS_DEBUG_BUILD && !m_isDebugRegistered)
			{
				BuildDebugMapRuns();
				DebugWindow::Get().Register(DEBUG_LABEL, [this] { DrawDebug(); });
				m_isDebugRegistered = true;
			}
		}


		void OrcaManager::Update()
		{
			for (auto& data : m_orcaList)
			{
				if (data.orca)
				{
					BEAST_PROFILE_SCOPE(u8"シャチ本体");
					data.orca->UpdateWrapper();
				}
				if (data.controller)
				{
					BEAST_PROFILE_SCOPE(u8"シャチAI");
					data.controller->Update();
				}
			}
		}


		void OrcaManager::UpdateModelOnly()
		{
			for (auto& data : m_orcaList)
			{
				if (data.orca) data.orca->UpdateModelOnly();
			}
		}


		bool OrcaManager::IsAllModelReady() const
		{
			for (const auto& data : m_orcaList)
			{
				if (data.orca && !data.orca->IsModelReady()) return false;
			}
			return true;
		}


		void OrcaManager::Render(RenderContext& rc)
		{
			for (auto& data : m_orcaList)
			{
				if (data.orca) data.orca->RenderWrapper(rc);
			}
		}


		void OrcaManager::NotifyVictimGhostHidden(int orcaId)
		{
			for (auto& data : m_orcaList)
			{
				if (data.orca != nullptr && data.controller != nullptr && data.orca->GetLogId() == orcaId)
				{
					data.controller->OnVictimGhostHidden();
					return;
				}
			}
		}


		bool OrcaManager::FindNearestChaserOf(const ChildPenguin* penguin, Vector3& outPos) const
		{
			if (penguin == nullptr) return false;
			float nearestDistSq = FLT_MAX;
			bool found = false;
			for (const auto& data : m_orcaList)
			{
				if (data.orca == nullptr || data.controller == nullptr) continue;
				if (data.controller->GetFoundPenguin() != penguin) continue;

				const Vector3 mouth = data.orca->GetMouthPosition();
				Vector3 diff = mouth - penguin->GetTransform().m_position;
				diff.y = 0.0f;
				if (diff.LengthSq() < nearestDistSq)
				{
					nearestDistSq = diff.LengthSq();
					outPos = mouth;
					found = true;
				}
			}
			return found;
		}


		std::vector<Orca*> OrcaManager::GetOrcas() const
		{
			std::vector<Orca*> list;
			for (const auto& data : m_orcaList) list.push_back(data.orca);
			return list;
		}


		std::vector<OrcaController*> OrcaManager::GetControllers() const
		{
			std::vector<OrcaController*> list;
			for (const auto& data : m_orcaList) list.push_back(data.controller);
			return list;
		}


		//============================================//
		// デバッグ表示
		//============================================//

		void OrcaManager::BuildDebugMapRuns()
		{
			m_debugMapRuns.clear();
			const int size = m_seaGrid.GetSize();
			if (size <= 0) return;

			const int cellsPerPixel = max(1, (size + DEBUG_MAP_MAX_PIXELS - 1) / DEBUG_MAP_MAX_PIXELS);
			m_debugMapPixels = (size + cellsPerPixel - 1) / cellsPerPixel;

			for (int py = 0; py < m_debugMapPixels; py++)
			{
				// 画面の上が +Z（北）
				const int cz0 = (m_debugMapPixels - 1 - py) * cellsPerPixel;
				int runStart = 0;
				uint32_t runColor = 0;
				for (int px = 0; px <= m_debugMapPixels; px++)
				{
					uint32_t color = 0;
					if (px < m_debugMapPixels)
					{
						// 1画素に入るセルのうち、泳げる＞海＞それ以外 の順で色を決める
						bool anyNavigable = false;
						bool anySea = false;
						const int cx0 = px * cellsPerPixel;
						for (int dz = 0; dz < cellsPerPixel; dz++)
						{
							for (int dx = 0; dx < cellsPerPixel; dx++)
							{
								anyNavigable |= m_seaGrid.IsNavigableCell(cx0 + dx, cz0 + dz);
								anySea |= m_seaGrid.IsSeaCell(cx0 + dx, cz0 + dz);
							}
						}
						color = anyNavigable ? MAP_COLOR_NAVIGABLE : (anySea ? MAP_COLOR_SEA : MAP_COLOR_LAND);
					}
					if (px == 0)
					{
						runColor = color;
						continue;
					}
					if (px == m_debugMapPixels || color != runColor)
					{
						m_debugMapRuns.push_back({ runStart, px, py, runColor });
						runStart = px;
						runColor = color;
					}
				}
			}
		}


		void OrcaManager::DrawDebug()
		{
			ImGui::Text(u8"セルフテスト: %s", m_selfTestMessage.c_str());
			ImGui::Text(u8"地図: %d×%d セル（セル %.0f）、航行可能 %d セル、作成 %.1f ms",
				m_seaGrid.GetSize(), m_seaGrid.GetSize(), m_seaGrid.GetSettings().cellSize,
				m_seaGrid.GetNavigableCellCount(), m_buildMilliseconds);
			ImGui::Text(u8"条件: 海底 %.1f 以下 / 余白 %.1f / 外周 %.0f",
				m_seaGrid.GetSettings().seaFloorMaxY, m_seaGrid.GetSettings().bodyClearance, m_seaGrid.GetSettings().boundaryRadius);
			if (m_snappedPointCount > 0)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), u8"泳げる場所の外にあったので寄せた点: %d", m_snappedPointCount);
			}

			// 親ペンギンの位置（巡回点を決めるときに読む）
			if (auto* cpm = ChildPenguinManager::GetInstance())
			{
				const Vector3 daddy = cpm->GetDaddyPosition();
				ImGui::Text(u8"親ペンギン: (%.0f, %.0f) 航行可能=%s",
					daddy.x, daddy.z, m_seaGrid.IsNavigable(daddy) ? u8"はい" : u8"いいえ");
			}

			for (const auto& data : m_orcaList)
			{
				if (data.orca == nullptr || data.controller == nullptr) continue;
				const OrcaStateMachine* sm = data.orca->GetOrcaStateMachine();
				const Vector3& p = sm->GetTransform().m_position;
				ImGui::SeparatorText((std::string(u8"シャチ ") + std::to_string(data.orca->GetLogId())).c_str());
				ImGui::Text(u8"状態: %s  位置: (%.0f, %.0f)  深さ: %.0f  速さ: %.0f",
					OrcaController::GetStateName(data.controller->GetState()), p.x, p.z, sm->GetDepth(), sm->GetCurrentSpeed());
				ImGui::Text(u8"航行可能: %s  海から出かけた回数: %d  経路の点: %d",
					m_seaGrid.IsNavigable(p) ? u8"はい" : u8"いいえ", data.orca->GetSeaViolationCount(), data.controller->GetPathPointCount());
				const OrcaStatus* status = data.orca->GetStatus<OrcaStatus>();
				ImGui::Text(u8"スタミナ: %.1f  食べた数: %d",
					status ? status->GetStamina() : 0.0f, data.controller->GetEatCount());

				// 試験用のボタン（ImGui のIDが重ならないようシャチごとに区別する）
				ImGui::PushID(data.orca->GetLogId());
				if (ImGui::Button(u8"幽霊デバフ（巡回へ戻る）")) data.controller->DebugTriggerGhostDebuff();
				ImGui::SameLine();
				if (ImGui::Button(u8"スタミナを0に")) data.controller->DebugDrainStamina();
				if (ImGui::Button(u8"最寄りの陸の方向へノックバック")) data.controller->DebugKnockbackTowardLand();
				ImGui::PopID();
			}

			// --- 上から見た地図 ---
			if (m_debugMapPixels <= 0) return;
			const float scale = 2.0f; // 1画素を2×2で描く
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImVec2 origin = ImGui::GetCursorScreenPos();
			for (const MapRun& run : m_debugMapRuns)
			{
				drawList->AddRectFilled(
					ImVec2(origin.x + run.x0 * scale, origin.y + run.y * scale),
					ImVec2(origin.x + run.x1 * scale, origin.y + (run.y + 1) * scale),
					run.color);
			}

			const float worldSize = m_seaGrid.GetSize() * m_seaGrid.GetSettings().cellSize;
			const float mapSize = m_debugMapPixels * scale;
			auto toMap = [&](const Vector3& world)
			{
				const float u = (world.x - m_seaGrid.GetOrigin()) / worldSize;
				const float v = (world.z - m_seaGrid.GetOrigin()) / worldSize;
				return ImVec2(origin.x + u * mapSize, origin.y + (1.0f - v) * mapSize);
			};

			for (const auto& data : m_orcaList)
			{
				if (data.orca == nullptr || data.controller == nullptr) continue;
				for (const Vector3& point : data.controller->GetPatrolPoints())
				{
					drawList->AddCircleFilled(toMap(point), 3.0f, MAP_COLOR_PATROL);
				}
				drawList->AddCircleFilled(toMap(data.orca->GetOrcaStateMachine()->GetTransform().m_position), 4.0f, MAP_COLOR_ORCA);
			}
			if (auto* cpm = ChildPenguinManager::GetInstance())
			{
				drawList->AddCircle(toMap(cpm->GetDaddyPosition()), 4.0f, MAP_COLOR_DADDY, 0, 2.0f);
			}
			ImGui::Dummy(ImVec2(mapSize, mapSize));
		}
	}
}
