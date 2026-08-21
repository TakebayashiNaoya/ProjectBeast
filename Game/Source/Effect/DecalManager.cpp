/**
 * @file DecalManager.cpp
 * @brief デカールを管理するクラス
 * @author 立山
 */
#include "stdafx.h"

#include "DecalManager.h"
#include "DecalProfiler.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/StageSystem.h"

#include <cstdlib>


namespace {
	const wchar_t* TEX_PATH_SNOW_FOOTPRINT = L"Assets/effect/decal/snowFootprint.DDS";
	const wchar_t* TEX_PATH_GRASS_FOOTPRINT = L"Assets/effect/decal/grassFootprint.DDS";
	const wchar_t* TEX_PATH_ROCK_FOOTPRINT = L"Assets/effect/decal/rockFootprint.DDS";
	const wchar_t* TEX_PATH_BEAR_FOOTPRINT = L"Assets/effect/decal/bearFootprint.DDS";
	const char* SHARED_QUAD_TKM_KEY = "decal_shared_quad";

	const Vector4 COLOR_GRASS_FOOTPRINT(0.35f, 0.42f, 0.20f, 1.0f);
	const Vector4 COLOR_ROCK_FOOTPRINT(0.35f, 0.35f, 0.35f, 1.0f);
	const Vector4 COLOR_SNOW_FOOTPRINT(0.55f, 0.70f, 0.90f, 1.0f);


	/** @brief 環境変数を整数で読む。未設定なら既定値を返す */
	int GetEnvInt(const char* name, int defaultValue) {
		char*  value = nullptr;
		size_t length = 0;
		if (_dupenv_s(&value, &length, name) != 0 || value == nullptr) return defaultValue;
		const int result = std::atoi(value);
		free(value);
		return result;
	}

	// ベンチマークの足跡をばらまく円のパラメータ。
	// 親ペンギンの初期位置(0,140,0)の周りの氷盤の上に収まる範囲を選んでいる
	constexpr float BENCH_RADIUS_MIN = 200.0f;
	constexpr float BENCH_RADIUS_MAX = 900.0f;
	/** 1点ごとに回す角度。黄金角にして同じ場所を続けて踏まないようにする */
	constexpr float BENCH_ANGLE_STEP = 2.39996f;
	// 生存時間は CharacterBase.cpp の FOOTPRINT_LIFE_SECONDS / FADE_OUT_SECONDS に合わせる。
	// プール内に同時に残る枚数が変わると、スロットの取り合い方まで変わってしまうため
	constexpr float BENCH_LIFE_SECONDS = 1.0f;
	constexpr float BENCH_FADE_OUT_SECONDS = 0.5f;
}

namespace app {
	namespace effect {
		TerrainHeightInfo DecalManager::BuildTerrainHeightInfo() const {
			TerrainHeightInfo info;
			if (auto* stageSystem = actor::StageSystem::GetInstance()) {
				if (auto* terrain = stageSystem->GetTerrain()) {
					info.heightmapTex = &terrain->GetHeightmapTextureGpu();
					info.halfWidth = terrain->GetHalfWidth();
					info.halfDepth = terrain->GetHalfDepth();
					info.heightScale = terrain->GetHeightScale();
					info.yOffset = terrain->GetYOffset();
				}
			}
			return info;
		}


		void DecalManager::EnsureInited() {
			if (m_isInited) return;

			m_snowFootprintTex.InitFromDDSFile(TEX_PATH_SNOW_FOOTPRINT);
			m_grassFootprintTex.InitFromDDSFile(TEX_PATH_GRASS_FOOTPRINT);
			m_rockFootprintTex.InitFromDDSFile(TEX_PATH_ROCK_FOOTPRINT);
			m_bearFootprintTex.InitFromDDSFile(TEX_PATH_BEAR_FOOTPRINT);

			// 種類ごとにプールを持つ。ここで作るのは ModelRender の器だけで、
			// 中身（InitFromLoaded）は最初にそのスロットを使うときまで遅らせる。
			// 空のままのスロットは一度も作られないので、枚数を増やしても起動は重くならない
			for (auto& pool : m_decalPools) {
				pool.reserve(MAX_DECAL_NUM_PER_KIND);
				for (int i = 0; i < MAX_DECAL_NUM_PER_KIND; ++i) {
					pool.emplace_back();
					pool.back().Prepare();
				}
			}


			if (g_engine->GetTkmFileFromBank(SHARED_QUAD_TKM_KEY) == nullptr) {
				std::vector<TkmFile::SVertex> vertices(4);
				// 板の4頂点。XZ平面上（Y=0）に配置
				vertices[0].pos = Vector3(-0.5f, 0.5f, 0.0f); // 前左
				vertices[1].pos = Vector3(0.5f, 0.5f, 0.0f); // 前右
				vertices[2].pos = Vector3(-0.5f, -0.5f, 0.0f); // 後左
				vertices[3].pos = Vector3(0.5f, -0.5f, 0.0f); // 後右

				// 旧コードは全頂点UVが(0,0)固定でテクスチャの絵柄が
				// 正しく貼られない不具合があった。四隅に0～1のUVを割り当てる。
				vertices[0].uv = Vector2(1.0f, 1.0f);
				vertices[1].uv = Vector2(0.0f, 1.0f);
				vertices[2].uv = Vector2(1.0f, 0.0f);
				vertices[3].uv = Vector2(0.0f, 0.0f);

				for (auto& v : vertices) {
					v.normal = Vector3::Up; v.tangent = Vector3::Right; v.binormal = Vector3::Front;
					v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0; v.skinWeights = Vector4{ 0,0,0,0 };
				}

				std::vector<uint32_t> indices = { 2, 3, 0,  3, 1, 0 }; // 上向き1面のみ

				TkmFile::SIndexBuffer32 ib; ib.indices = std::move(indices);
				TkmFile::SMesh mesh; mesh.isFlatShading = false;
				mesh.materials.push_back(TkmFile::SMaterial{});
				mesh.vertexBuffer = std::move(vertices);
				mesh.indexBuffer32Array.push_back(std::move(ib));

				std::vector<TkmFile::SMesh> meshes; meshes.push_back(std::move(mesh));
				auto* tkm = new nsK2EngineLow::TkmFile();
				tkm->Build(std::move(meshes));
				g_engine->ReplaceTkmFileInBank(SHARED_QUAD_TKM_KEY, tkm);
			}
			m_isInited = true;
		}


		nsK2EngineLow::Texture* DecalManager::GetTextureForKind(DecalKind kind) {
			switch (kind) {
			case DecalKind::GrassFootprint: return &m_grassFootprintTex;
			case DecalKind::RockFootprint:  return &m_rockFootprintTex;
			case DecalKind::BearFootprint:  return &m_bearFootprintTex;
			case DecalKind::SnowFootprint: default: return &m_snowFootprintTex;
			}
		}


		DecalManager::GroundHitInfo DecalManager::RaycastGround(const Vector3& fromPosXZ) const {
			GroundHitInfo result;
			const Vector3 rayStart(fromPosXZ.x, fromPosXZ.y + RAY_START_HEIGHT, fromPosXZ.z);
			const Vector3 rayEnd = rayStart + (Vector3(0, -1, 0) * RAY_MAX_DISTANCE);
			nsBeastEngine::nsCollision::RaycastHit hit;

			if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit, nsBeastEngine::nsCollision::ALL_COLLISION_ATTRIBUTE_MASK)) {
				result.isHit = true;
				// 板(Quad)は地形の表面とぴったり同じ高さに置くと
				// Zファイティングで消えてしまうため、法線方向にわずかに浮かせる
				result.position = Vector3(hit.point.x, hit.point.y, hit.point.z) + hit.normal * PROJECTED_SURFACE_OFFSET;
				result.normal = hit.normal;
			}
			return result;
		}


		void DecalManager::SpawnFootprint(const Vector3& position, float yawRadian, DecalKind kind, float size, float lifeSeconds, float fadeOutSeconds, const Vector4& color, bool autoDetectSurface, int priority) {
			auto& profiler = DecalProfiler::Get();
			const bool   isProfiling = profiler.IsEnabled();
			const double spawnBeginMs = isProfiling ? DecalProfiler::NowMs() : 0.0;
			profiler.RecordSpawnCall();

			EnsureInited();

			const double raycastBeginMs = isProfiling ? DecalProfiler::NowMs() : 0.0;
			const GroundHitInfo hit = RaycastGround(position);
			const double raycastMs = isProfiling ? (DecalProfiler::NowMs() - raycastBeginMs) : 0.0;
			if (!hit.isHit) {
				profiler.RecordSpawnNoGround();
				return;
			}

			DecalKind finalKind = kind; Vector4 finalColor = color;
			if (autoDetectSurface) {
				if (auto* stageSystem = actor::StageSystem::GetInstance()) {
					if (auto* terrain = stageSystem->GetTerrain()) {
						switch (terrain->GetSurfaceTypeAt(hit.position)) {
						case actor::TerrainObject::SurfaceType::Grass: finalKind = DecalKind::GrassFootprint; finalColor = COLOR_GRASS_FOOTPRINT; break;
						case actor::TerrainObject::SurfaceType::Rock:  finalKind = DecalKind::RockFootprint;  finalColor = COLOR_ROCK_FOOTPRINT; break;
						case actor::TerrainObject::SurfaceType::Snow: default: finalKind = DecalKind::SnowFootprint; finalColor = COLOR_SNOW_FOOTPRINT; break;
						}
					}
				}
			}

			// 枠は必ず「その種類のプール」から取る。こうすると Decal::Spawn の
			// m_kind != kind が二度と成立せず、ModelRender の作り直しが起きない
			std::vector<Decal>& pool = m_decalPools[static_cast<int>(finalKind)];

			int targetIndex = -1; float minRemaining = FLT_MAX;
			if (priority == 0) {
				int childCount = 0;
				for (const auto& decal : pool) { if (decal.IsActive() && decal.GetPriority() == 0) childCount++; }
				if (childCount >= MAX_CHILD_DECAL_NUM_PER_KIND) {
					for (int i = 0; i < static_cast<int>(pool.size()); ++i) {
						if (pool[i].IsActive() && pool[i].GetPriority() == 0) {
							if (pool[i].GetRemainingLife() < minRemaining) { minRemaining = pool[i].GetRemainingLife(); targetIndex = i; }
						}
					}
				}
			}
			if (targetIndex < 0) {
				for (int i = 0; i < static_cast<int>(pool.size()); ++i) {
					if (!pool[i].IsActive()) { targetIndex = i; break; }
					if (pool[i].GetPriority() <= priority) {
						if (pool[i].GetRemainingLife() < minRemaining) { minRemaining = pool[i].GetRemainingLife(); targetIndex = i; }
					}
				}
			}
			if (targetIndex < 0) {
				profiler.RecordSpawnNoSlot();
				return;
			}
			// まだ生きているデカールを潰した場合は記録する。プールの枚数が
			// 足りているかどうかをあとから数字で確かめられるようにするため
			if (pool[targetIndex].IsActive()) profiler.RecordEviction();

			const TerrainHeightInfo terrainInfo = BuildTerrainHeightInfo();

			pool[targetIndex].Spawn(hit.position, hit.normal, yawRadian, size, finalKind, GetTextureForKind(finalKind), finalColor,
				lifeSeconds, fadeOutSeconds, priority, SHARED_QUAD_TKM_KEY, terrainInfo);

			if (isProfiling) {
				profiler.RecordSpawnAccepted(DecalProfiler::NowMs() - spawnBeginMs, raycastMs);
			}
		}


		void DecalManager::Update() {
			UpdateBenchmark();

			if (!m_isInited) return;

			auto& profiler = DecalProfiler::Get();
			const bool   isProfiling = profiler.IsEnabled();
			const double beginMs = isProfiling ? DecalProfiler::NowMs() : 0.0;

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			for (auto& pool : m_decalPools) {
				for (auto& decal : pool) decal.Update(deltaTime);
			}

			if (isProfiling) profiler.RecordUpdate(DecalProfiler::NowMs() - beginMs);
		}


		void DecalManager::Render(RenderContext& rc) {
			auto& profiler = DecalProfiler::Get();
			const bool   isProfiling = profiler.IsEnabled();
			const double beginMs = isProfiling ? DecalProfiler::NowMs() : 0.0;

			int drawCalls = 0;
			int activeDecals = 0;
			int poolSize = 0;
			if (m_isInited) {
				for (auto& pool : m_decalPools) {
					poolSize += static_cast<int>(pool.size());
					for (auto& decal : pool) {
						if (decal.IsActive()) { activeDecals++; drawCalls++; }
						decal.Render(rc);
					}
				}
			}

			// フレームの最後にデカールを描いているので、ここを1フレームの区切りにする。
			// 地形のロード前（タイトル画面）も通るため、素の状態のフレーム時間も残る
			if (isProfiling) {
				profiler.EndFrame(DecalProfiler::NowMs() - beginMs, drawCalls, activeDecals, poolSize);
			}
		}


		void DecalManager::UpdateBenchmark() {
			// 初回だけ環境変数を読む
			static bool isConfigLoaded = false;
			if (!isConfigLoaded) {
				isConfigLoaded = true;
				m_isBenchEnabled = (GetEnvInt("DECAL_BENCH", 0) != 0);
				m_benchFramesPerPhase = GetEnvInt("DECAL_BENCH_FRAMES", 600);
				// 実測（Normal・5400フレーム）の受理済みスポーン数 7.1回/秒 に合わせた既定値。
				// 負荷を意図的に上げたいときだけ環境変数で増やす
				m_benchSpawnsPerSecond = static_cast<float>(GetEnvInt("DECAL_BENCH_SPAWNS_PER_SEC", 12));
				m_isBenchQuitOnFinish = (GetEnvInt("DECAL_BENCH_QUIT", 1) != 0);
			}
			if (!m_isBenchEnabled || m_benchPhase == BenchPhase::Finished) return;

			// 地形が出来上がるまでは何もしない（レイキャストが当たらず計測にならない）
			auto* stageSystem = actor::StageSystem::GetInstance();
			if (stageSystem == nullptr || stageSystem->GetTerrain() == nullptr) return;

			auto& profiler = DecalProfiler::Get();

			m_benchFrameInPhase++;
			if (m_benchFrameInPhase > m_benchFramesPerPhase) {
				m_benchFrameInPhase = 0;
				switch (m_benchPhase) {
				case BenchPhase::Warmup:     m_benchPhase = BenchPhase::SingleKind; break;
				case BenchPhase::SingleKind: m_benchPhase = BenchPhase::MixedKind;  break;
				case BenchPhase::MixedKind:  m_benchPhase = BenchPhase::AutoDetect; break;
				case BenchPhase::AutoDetect: m_benchPhase = BenchPhase::Finished;   break;
				default: break;
				}
			}

			switch (m_benchPhase) {
			case BenchPhase::Warmup:     profiler.SetPhase(0, "warmup");     break;
			case BenchPhase::SingleKind: profiler.SetPhase(1, "singleKind"); break;
			case BenchPhase::MixedKind:  profiler.SetPhase(2, "mixedKind");  break;
			case BenchPhase::AutoDetect: profiler.SetPhase(3, "autoDetect"); break;
			case BenchPhase::Finished:
				profiler.SetPhase(4, "finished");
				profiler.Flush();
				if (m_isBenchQuitOnFinish) PostQuitMessage(0);
				return;
			}

			if (m_benchPhase == BenchPhase::Warmup) return;

			// 種類ごとに1枚ずつテクスチャを使い分けているので、混ぜる区間では
			// 雪とクマを交互に出す。実際のプレイでも、ペンギン（自動判定）と
			// シロクマ（BearFootprint固定）が同じプールを取り合っている
			static const DecalKind MIXED_SEQUENCE[] = {
				DecalKind::SnowFootprint, DecalKind::BearFootprint,
				DecalKind::RockFootprint, DecalKind::BearFootprint,
			};

			// 実時間ベースで本数を決める。フレームレートが変わっても
			// 「1秒あたり何枚出したか」が揃うようにする
			m_benchSpawnAccumulator += m_benchSpawnsPerSecond * g_gameTime->GetFrameDeltaTime();
			const int spawnNum = static_cast<int>(m_benchSpawnAccumulator);
			m_benchSpawnAccumulator -= static_cast<float>(spawnNum);

			for (int i = 0; i < spawnNum; ++i) {
				const int   n = m_benchSpawnCounter++;
				const float angle = BENCH_ANGLE_STEP * n;
				const float radius = BENCH_RADIUS_MIN
					+ (BENCH_RADIUS_MAX - BENCH_RADIUS_MIN) * (0.5f + 0.5f * sinf(n * 0.137f));
				const Vector3 position(radius * cosf(angle), 140.0f, radius * sinf(angle));

				switch (m_benchPhase) {
				case BenchPhase::SingleKind:
					SpawnFootprint(position, angle, DecalKind::SnowFootprint,
						DEFAULT_FOOTPRINT_SIZE, BENCH_LIFE_SECONDS, BENCH_FADE_OUT_SECONDS,
						COLOR_SNOW_FOOTPRINT, false, DEFAULT_PRIORITY);
					break;

				case BenchPhase::MixedKind:
					SpawnFootprint(position, angle, MIXED_SEQUENCE[n % _countof(MIXED_SEQUENCE)],
						DEFAULT_FOOTPRINT_SIZE, BENCH_LIFE_SECONDS, BENCH_FADE_OUT_SECONDS,
						DEFAULT_FOOTPRINT_COLOR, false, DEFAULT_PRIORITY);
					break;

				case BenchPhase::AutoDetect:
					SpawnFootprint(position, angle, DecalKind::SnowFootprint,
						DEFAULT_FOOTPRINT_SIZE, BENCH_LIFE_SECONDS, BENCH_FADE_OUT_SECONDS,
						DEFAULT_FOOTPRINT_COLOR, true, DEFAULT_PRIORITY);
					break;

				default: break;
				}
			}
		}
	}
}