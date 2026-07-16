/**
 * @file DecalManager.cpp
 * @brief Decal（本体）の生成・更新・描画をまとめて管理するマネージャー
 */
#include "stdafx.h"
#include "DecalManager.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/StageSystem.h"   // 地形（TerrainObject）を取得するため


namespace
{
	/** @brief 足跡テクスチャパス（実際の格納場所に合わせて変更してください） */
	const wchar_t* TEX_PATH_SNOW_FOOTPRINT = L"Assets/effect/decal/snowFootprint.DDS";
	const wchar_t* TEX_PATH_GRASS_FOOTPRINT = L"Assets/effect/decal/grassFootprint.DDS";
	const wchar_t* TEX_PATH_ROCK_FOOTPRINT = L"Assets/effect/decal/rockFootprint.DDS";
}


namespace app
{
	namespace effect
	{
		void DecalManager::EnsureInited()
		{
			if (m_isInited) return;

			// ★投影デカールは呼び出しのたびに専用メッシュを組み立てるため、
			//   共有クアッドメッシュの事前生成は不要になった

			m_snowFootprintTex.InitFromDDSFile(TEX_PATH_SNOW_FOOTPRINT);
			m_grassFootprintTex.InitFromDDSFile(TEX_PATH_GRASS_FOOTPRINT);
			m_rockFootprintTex.InitFromDDSFile(TEX_PATH_ROCK_FOOTPRINT);

			// プール分のDecalインスタンスを事前確保しておく（毎回のnew/deleteを避ける）
			m_decals.resize(MAX_DECAL_NUM);
			for (int i = 0; i < static_cast<int>(m_decals.size()); ++i)
			{
				// ★各スロットに専用バンクキーを割り当てる（"decal_patch_0"など）
				m_decals[i].Prepare(i);
			}

			m_isInited = true;
		}


		nsK2EngineLow::Texture* DecalManager::GetTextureForKind(DecalKind kind)
		{
			switch (kind)
			{
			case DecalKind::GrassFootprint: return &m_grassFootprintTex;
			case DecalKind::RockFootprint:  return &m_rockFootprintTex;
			case DecalKind::SnowFootprint:
			default:                        return &m_snowFootprintTex;
			}
		}


		DecalManager::GroundHitInfo DecalManager::RaycastGround(const Vector3& fromPosXZ) const
		{
			GroundHitInfo result;
			const Vector3 rayStart(fromPosXZ.x, fromPosXZ.y + RAY_START_HEIGHT, fromPosXZ.z);
			const Vector3 rayDir(0.0f, -1.0f, 0.0f);
			nsBeastEngine::nsCollision::RaycastHit hit;
			const Vector3 rayEnd = rayStart + (rayDir * RAY_MAX_DISTANCE);

			const bool isHit = nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(
				rayStart,
				rayEnd,
				hit,
				nsBeastEngine::nsCollision::ALL_COLLISION_ATTRIBUTE_MASK
			);

			if (isHit)
			{
				result.isHit = true;
				result.position = Vector3(hit.point.x, hit.point.y + 1.0f, hit.point.z);
				result.normal = hit.normal;
			}
			return result;
		}


		std::vector<Vector3> DecalManager::BuildProjectedGridPositions(
			const Vector3& center,
			float yawRadian,
			float size,
			actor::TerrainObject* terrain,
			int gridResolution) const
		{
			std::vector<Vector3> positions;
			positions.reserve(static_cast<size_t>(gridResolution) * gridResolution);

			const float cosYaw = cosf(yawRadian);
			const float sinYaw = sinf(yawRadian);

			for (int j = 0; j < gridResolution; ++j)
			{
				for (int i = 0; i < gridResolution; ++i)
				{
					const float u = (gridResolution > 1) ? float(i) / float(gridResolution - 1) : 0.5f;
					const float v = (gridResolution > 1) ? float(j) / float(gridResolution - 1) : 0.5f;

					// 中心を原点とした未回転のローカルXZオフセット
					// ★修正: v=0(テクスチャのつま先側)が前方、v=1(かかと側)が後方になるよう反転
					const float lx = (u - 0.5f) * size;
					const float lz = (0.5f - v) * size;

					// 進行方向(yaw)に合わせて回転させてからワールド座標へ
					const float wx = center.x + (lx * cosYaw + lz * sinYaw);
					const float wz = center.z + (-lx * sinYaw + lz * cosYaw);

					// 地形が取得できればその高さに、できなければレイキャスト位置の高さにフォールバック
					float wy = center.y;
					if (terrain != nullptr)
					{
						wy = terrain->GetHeightAt(Vector3(wx, 0.0f, wz)) + PROJECTED_SURFACE_OFFSET;
					}

					positions.emplace_back(wx, wy, wz);
				}
			}

			return positions;
		}


		void DecalManager::SpawnFootprint(
			const Vector3& position,
			float yawRadian,
			DecalKind kind,
			float size,
			float lifeSeconds,
			float fadeOutSeconds,
			const Vector4& color,
			bool autoDetectSurface)
		{
			EnsureInited();

			const GroundHitInfo hit = RaycastGround(position);
			if (!hit.isHit) return;

			actor::TerrainObject* terrain = nullptr;
			if (auto* stageSystem = actor::StageSystem::GetInstance())
			{
				terrain = stageSystem->GetTerrain();
			}

			// ★地形の自動判定でkind/colorを上書きする
			DecalKind finalKind = kind;
			Vector4 finalColor = color;

			if (autoDetectSurface && terrain != nullptr)
			{
				switch (terrain->GetSurfaceTypeAt(hit.position))
				{
				case actor::TerrainObject::SurfaceType::Grass:
					finalKind = DecalKind::GrassFootprint;
					finalColor = Vector4(0.35f, 0.42f, 0.20f, 1.0f);
					break;
				case actor::TerrainObject::SurfaceType::Rock:
					finalKind = DecalKind::RockFootprint;
					finalColor = Vector4(0.35f, 0.35f, 0.35f, 1.0f);
					break;
				case actor::TerrainObject::SurfaceType::Snow:
				default:
					finalKind = DecalKind::SnowFootprint;
					finalColor = Vector4(0.55f, 0.70f, 0.90f, 1.0f);
					break;
				}
			}

			// 空きスロットを探す。なければ最も残り時間が少ないものを再利用する。
			int targetIndex = -1;
			float minRemaining = FLT_MAX;
			for (int i = 0; i < static_cast<int>(m_decals.size()); ++i)
			{
				if (!m_decals[i].IsActive())
				{
					targetIndex = i;
					break;
				}
				if (m_decals[i].GetRemainingLife() < minRemaining)
				{
					minRemaining = m_decals[i].GetRemainingLife();
					targetIndex = i;
				}
			}
			if (targetIndex < 0) return;

			// ★投影デカール: 地形の凹凸に沿った格子頂点を組み立ててから設置する
			const std::vector<Vector3> gridPositions = BuildProjectedGridPositions(
				hit.position, yawRadian, size, terrain, GRID_RESOLUTION);

			m_decals[targetIndex].SetupProjected(
				gridPositions, GRID_RESOLUTION, GetTextureForKind(finalKind), finalColor);
			m_decals[targetIndex].SetLife(lifeSeconds, fadeOutSeconds);
		}


		void DecalManager::Update()
		{
			if (!m_isInited) return;

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			for (auto& decal : m_decals)
			{
				if (decal.IsActive())
				{
					decal.Update(deltaTime);
				}
			}
		}


		void DecalManager::Render(RenderContext& rc)
		{
			if (!m_isInited) return;

			for (auto& decal : m_decals)
			{
				decal.Render(rc);
			}
		}
	}
}
