/**
 * @file DecalManager.cpp
 * @brief デカールを管理するクラス
 * @author 立山
 */
#include "stdafx.h"

#include "DecalManager.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/StageSystem.h"


namespace {
	const wchar_t* TEX_PATH_SNOW_FOOTPRINT = L"Assets/effect/decal/snowFootprint.DDS";
	const wchar_t* TEX_PATH_GRASS_FOOTPRINT = L"Assets/effect/decal/grassFootprint.DDS";
	const wchar_t* TEX_PATH_ROCK_FOOTPRINT = L"Assets/effect/decal/rockFootprint.DDS";
	const wchar_t* TEX_PATH_BEAR_FOOTPRINT = L"Assets/effect/decal/bearFootprint.DDS";
	const char* SHARED_QUAD_TKM_KEY = "decal_shared_quad";

	const Vector4 COLOR_GRASS_FOOTPRINT(0.35f, 0.42f, 0.20f, 1.0f);
	const Vector4 COLOR_ROCK_FOOTPRINT(0.35f, 0.35f, 0.35f, 1.0f);
	const Vector4 COLOR_SNOW_FOOTPRINT(0.55f, 0.70f, 0.90f, 1.0f);



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

			m_decals.reserve(MAX_DECAL_NUM);
			for (int i = 0; i < MAX_DECAL_NUM; ++i) {
				m_decals.emplace_back();
				m_decals.back().Prepare();
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
			EnsureInited();

			const GroundHitInfo hit = RaycastGround(position);
			if (!hit.isHit) return;

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

			int targetIndex = -1; float minRemaining = FLT_MAX;
			if (priority == 0) {
				int childCount = 0;
				for (const auto& decal : m_decals) { if (decal.IsActive() && decal.GetPriority() == 0) childCount++; }
				if (childCount >= MAX_CHILD_DECAL_NUM) {
					for (int i = 0; i < static_cast<int>(m_decals.size()); ++i) {
						if (m_decals[i].IsActive() && m_decals[i].GetPriority() == 0) {
							if (m_decals[i].GetRemainingLife() < minRemaining) { minRemaining = m_decals[i].GetRemainingLife(); targetIndex = i; }
						}
					}
				}
			}
			if (targetIndex < 0) {
				for (int i = 0; i < static_cast<int>(m_decals.size()); ++i) {
					if (!m_decals[i].IsActive()) { targetIndex = i; break; }
					if (m_decals[i].GetPriority() <= priority) {
						if (m_decals[i].GetRemainingLife() < minRemaining) { minRemaining = m_decals[i].GetRemainingLife(); targetIndex = i; }
					}
				}
			}
			if (targetIndex < 0) return;

			const TerrainHeightInfo terrainInfo = BuildTerrainHeightInfo();

			m_decals[targetIndex].Spawn(hit.position, hit.normal, yawRadian, size, finalKind, GetTextureForKind(finalKind), finalColor,
				lifeSeconds, fadeOutSeconds, priority, SHARED_QUAD_TKM_KEY, terrainInfo);
		}


		void DecalManager::Update() {
			if (!m_isInited) return;
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			for (auto& decal : m_decals) decal.Update(deltaTime);
		}


		void DecalManager::Render(RenderContext& rc) {
			if (!m_isInited) return;
			for (auto& decal : m_decals) decal.Render(rc);
		}
	}
}