/**
 * @file TerrainObject.cpp
 * @brief ハイトマップから生成する地形オブジェクト
 * @author 竹林
 */
#include "stdafx.h"
#include "TerrainObject.h"
#include "DirectXTex/DirectXTex.h"


namespace
{
	/** ハイトマップ */
	static const wchar_t* HEIGHTMAP_PATH  = L"Assets/modelData/stage/Terrain/TutorialStageHeightMap.dds";
	/** スプラットマップ */
	static const wchar_t* SPLATMAP_PATH   = L"Assets/modelData/stage/Terrain/TutorialStageSplatMap.dds";

	/** 物理コリジョン用フルメッシュのバンクキー */
	static const char* TERRAIN_TKM_KEY = "terrain_generated";
	/** チャンク描画用メッシュのバンクキープレフィックス（"terrain_chunk_0"〜） */
	static const char* TERRAIN_CHUNK_KEY_PREFIX = "terrain_chunk";

	/** 地形テクスチャ（スプラットマップの R=snow, G=grass, B=rock） */
	static const wchar_t* TEX_PATH_SNOW   = L"Assets/modelData/stage/Terrain/snow.DDS";
	static const wchar_t* TEX_PATH_GRASS  = L"Assets/modelData/stage/Terrain/grass.DDS";
	static const wchar_t* TEX_PATH_ROCK   = L"Assets/modelData/stage/Terrain/rock.DDS";

	/** PBR テクスチャ */
	static const wchar_t* TEX_PATH_SNOW_NORMAL     = L"Assets/modelData/stage/Terrain/snow_Normal.DDS";
	static const wchar_t* TEX_PATH_SNOW_ROUGHNESS  = L"Assets/modelData/stage/Terrain/snow_Roughness.DDS";
	static const wchar_t* TEX_PATH_GRASS_NORMAL    = L"Assets/modelData/stage/Terrain/grass_Normal.DDS";
	static const wchar_t* TEX_PATH_GRASS_ROUGHNESS = L"Assets/modelData/stage/Terrain/grass_Roughness.DDS";
	static const wchar_t* TEX_PATH_ROCK_NORMAL     = L"Assets/modelData/stage/Terrain/rock_Normal.DDS";
	static const wchar_t* TEX_PATH_ROCK_ROUGHNESS  = L"Assets/modelData/stage/Terrain/rock_Roughness.DDS";
}


namespace app
{
	namespace actor
	{
		void TerrainObject::Init(const TerrainConfig& config)
		{
			if (m_isInited) return;
			m_config = config;
			LoadHeightmap();
			GenerateMesh();
			InitRenderer();
			StartWrapper();
			m_isInited = true;
		}


		void TerrainObject::Update()
		{
			if (!m_isInited) return;
			m_modelRender.Update();
			for (auto& render : m_chunkRenders)
			{
				if (render) render->Update();
			}
		}


		void TerrainObject::Render(RenderContext& rc)
		{
			if (!m_isInited) return;
			const auto& frustum = g_renderingEngine->GetActiveFrustum();
			const int numChunks = static_cast<int>(m_chunkRenders.size());
			for (int i = 0; i < numChunks; i++)
			{
				if (!m_chunkRenders[i]) continue;
				if (!frustum.IsIntersectAABBWorld(m_chunkAABBs[i].GetMin(), m_chunkAABBs[i].GetMax())) {
					continue;
				}
				m_chunkRenders[i]->Draw(rc);
			}
		}


		void TerrainObject::LoadHeightmap()
		{
			// DirectXTex で DDS をロード（BC1〜BC7, BC6H を含む全圧縮フォーマット対応）
			DirectX::ScratchImage image;
			HRESULT hr = DirectX::LoadFromDDSFile(
				HEIGHTMAP_PATH, DirectX::DDS_FLAGS_NONE, nullptr, image);

			if (FAILED(hr))
			{
				m_heightmap.width = m_heightmap.height = 2;
				m_heightmap.pixels.assign(4, 0);
				return;
			}

			const DirectX::Image* img = image.GetImage(0, 0, 0);

			// ブロック圧縮フォーマット (BC1〜BC7, BC6H) はまず RGBA8 に展開する
			DirectX::ScratchImage decompressed;
			if (DirectX::IsCompressed(img->format))
			{
				hr = DirectX::Decompress(*img, DXGI_FORMAT_R8G8B8A8_UNORM, decompressed);
				if (FAILED(hr))
				{
					m_heightmap.width = m_heightmap.height = 2;
					m_heightmap.pixels.assign(4, 0);
					return;
				}
				img = decompressed.GetImage(0, 0, 0);
			}

			// R32_FLOAT 以外のフォーマットを R32_FLOAT に変換する（R チャンネルが高さ値）
			DirectX::ScratchImage converted;
			if (img->format != DXGI_FORMAT_R32_FLOAT)
			{
				hr = DirectX::Convert(
					*img, DXGI_FORMAT_R32_FLOAT,
					DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT,
					converted
				);
				if (FAILED(hr))
				{
					m_heightmap.width = m_heightmap.height = 2;
					m_heightmap.pixels.assign(4, 0);
					return;
				}
				img = converted.GetImage(0, 0, 0);
			}

			// img は R32_FLOAT に統一済み（1 pixel = 4 bytes = float 1 個）
			const int    srcW     = static_cast<int>(img->width);
			const int    srcH     = static_cast<int>(img->height);
			const int    rowPitch = static_cast<int>(img->rowPitch) / sizeof(float);
			const float* pixels   = reinterpret_cast<const float*>(img->pixels);

			m_heightmap.width  = srcW / m_config.subsample;
			m_heightmap.height = srcH / m_config.subsample;
			m_heightmap.pixels.assign(m_heightmap.width * m_heightmap.height, 0);

			for (int dstZ = 0; dstZ < m_heightmap.height; ++dstZ)
			{
				for (int dstX = 0; dstX < m_heightmap.width; ++dstX)
				{
					const float f = pixels[(dstZ * m_config.subsample) * rowPitch + (dstX * m_config.subsample)];
					m_heightmap.pixels[dstZ * m_heightmap.width + dstX] = static_cast<uint16_t>(std::clamp(f, 0.0f, 1.0f) * 65535.0f);
				}
			}
		}


		void TerrainObject::GenerateMesh()
		{
			const int W = m_heightmap.width;
			const int H = m_heightmap.height;

			// totalWidth/totalDepth から頂点間隔を算出（3ds Max Z-up 座標系）
			const float cellSizeX = (W > 1) ? m_config.totalWidth  / float(W - 1) : 1.0f;
			const float cellSizeZ = (H > 1) ? m_config.totalDepth  / float(H - 1) : 1.0f;
			const float originX   = -m_config.totalWidth  * 0.5f;
			const float originY   = -m_config.totalDepth  * 0.5f;  // 3ds Max の奥行き軸

			m_terrainCb.halfWidth   = m_config.totalWidth  * 0.5f;
			m_terrainCb.halfDepth   = m_config.totalDepth  * 0.5f;
			m_terrainCb.albedoScale = m_config.albedoScale;

			auto getH = [&](int px, int pz) -> float
			{
				px = max(0, min(W - 1, px));
				pz = max(0, min(H - 1, pz));
				return m_heightmap.pixels[pz * W + px] / 65535.0f * m_config.heightScale;
			};

			// 高さキャッシュ
			std::vector<float> heights(W * H);
			for (int z = 0; z < H; ++z)
				for (int x = 0; x < W; ++x)
					heights[z * W + x] = getH(x, z);

			// --------- 共通の TkmFile 構築ヘルパー ---------
			// [vxStart, vxEnd) × [vzStart, vzEnd) の頂点範囲と
			// [cxStart, cxEnd) × [czStart, czEnd) のセル範囲からメッシュを生成してバンクに登録する
			auto buildTkm = [&](
				int vxStart, int vxEnd, int vzStart, int vzEnd,
				int cxStart, int cxEnd, int czStart, int czEnd,
				const char* bankKey) -> nsK2EngineLow::TkmFile*
			{
				const int localW = vxEnd - vxStart;

				// ------ 頂点生成 ------
				std::vector<TkmFile::SVertex> vertices;
				vertices.reserve(localW * (vzEnd - vzStart));

				for (int vz = vzStart; vz < vzEnd; ++vz)
				{
					for (int vx = vxStart; vx < vxEnd; ++vx)
					{
						const float h  = heights[vz * W + vx];
						const float hR = getH(vx + 1, vz);
						const float hL = getH(vx - 1, vz);
						const float hU = getH(vx, vz + 1);
						const float hD = getH(vx, vz - 1);

						Vector3 dx = { 2.0f * cellSizeX, 0.0f, hR - hL };
						Vector3 dy = { 0.0f, 2.0f * cellSizeZ, hU - hD };
						Vector3 normal = Cross(dx, dy);
						normal.Normalize();

						TkmFile::SVertex v;
						v.pos         = Vector3(originX + vx * cellSizeX, originY + vz * cellSizeZ, h + m_config.yOffset);
						v.normal      = normal;
						v.tangent     = Vector3(1.0f, 0.0f, 0.0f);
						v.binormal    = Vector3(0.0f, 1.0f, 0.0f);
						v.uv          = Vector2(vx * m_config.uvTile, vz * m_config.uvTile);
						v.indices[0]  = v.indices[1] = v.indices[2] = v.indices[3] = 0;
						v.skinWeights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
						vertices.push_back(v);
					}
				}

				// ------ インデックス生成（各セル 2 三角形） ------
				std::vector<uint32_t> indices;
				indices.reserve((cxEnd - cxStart) * (czEnd - czStart) * 6);

				for (int cz = czStart; cz < czEnd; ++cz)
				{
					for (int cx = cxStart; cx < cxEnd; ++cx)
					{
						if (m_config.minHeight > 0.0f)
						{
							const float h0 = heights[ cz      * W +  cx     ];
							const float h1 = heights[ cz      * W + (cx + 1)];
							const float h2 = heights[(cz + 1) * W +  cx     ];
							const float h3 = heights[(cz + 1) * W + (cx + 1)];
							if (h0 < m_config.minHeight || h1 < m_config.minHeight ||
								h2 < m_config.minHeight || h3 < m_config.minHeight)
								continue;
						}

						// ローカル頂点インデックス（vxStart/vzStart 基点）
						const uint32_t lz = static_cast<uint32_t>(cz - vzStart);
						const uint32_t lx = static_cast<uint32_t>(cx - vxStart);
						const uint32_t i0 = lz * localW + lx;
						const uint32_t i1 = i0 + 1;
						const uint32_t i2 = (lz + 1) * localW + lx;
						const uint32_t i3 = i2 + 1;

						indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
						indices.push_back(i1); indices.push_back(i3); indices.push_back(i2);
					}
				}

				if (indices.empty()) return nullptr;  // minHeight で全クワッドが除外された場合

				// ------ TkmFile 構築 ------
				TkmFile::SMaterial mat = {};

				TkmFile::SIndexBuffer32 ib;
				ib.indices = std::move(indices);

				TkmFile::SMesh mesh;
				mesh.isFlatShading = false;
				mesh.materials.push_back(mat);
				mesh.vertexBuffer  = std::move(vertices);
				mesh.indexBuffer32Array.push_back(std::move(ib));

				std::vector<TkmFile::SMesh> meshes;
				meshes.push_back(std::move(mesh));

				auto* tkm = new TkmFile();
				tkm->Build(std::move(meshes));
				g_engine->ReplaceTkmFileInBank(bankKey, tkm);
				return tkm;
			};

			// --------- 物理用フルメッシュ ---------
			m_tkmFile = buildTkm(0, W, 0, H, 0, W - 1, 0, H - 1, TERRAIN_TKM_KEY);

			// --------- チャンク別描画メッシュ ---------
			const int chunkDiv    = m_config.chunkDivision;
			const int numCellsX   = W - 1;
			const int numCellsZ   = H - 1;
			// ceiling division：端のチャンクが小さくなることを許容する
			const int cpchX = (numCellsX + chunkDiv - 1) / chunkDiv;
			const int cpchZ = (numCellsZ + chunkDiv - 1) / chunkDiv;
			const int numChunks   = chunkDiv * chunkDiv;

			m_chunkTkmFiles.assign(numChunks, nullptr);
			m_chunkAABBs.resize(numChunks);

			for (int chunkZ = 0; chunkZ < chunkDiv; ++chunkZ)
			{
				for (int chunkX = 0; chunkX < chunkDiv; ++chunkX)
				{
					const int chunkIdx = chunkZ * chunkDiv + chunkX;

					// セル範囲
					const int cxStart = chunkX * cpchX;
					const int czStart = chunkZ * cpchZ;
					const int cxEnd   = min(cxStart + cpchX, numCellsX);
					const int czEnd   = min(czStart + cpchZ, numCellsZ);

					// 頂点範囲（セル範囲より1大きい）
					const int vxStart = cxStart;
					const int vzStart = czStart;
					const int vxEnd   = min(cxEnd + 1, W);
					const int vzEnd   = min(czEnd + 1, H);

					// ワールド空間 AABB を計算する
					// BeastModel は MakeRotationX(-PI/2) で 3ds Max Z-up → DX Y-up に変換する。
					// 変換後: world_x=src_x, world_y=src_z(高さ), world_z=-src_y(奥行き)
					float minH = FLT_MAX, maxH = -FLT_MAX;
					for (int vz = vzStart; vz < vzEnd; ++vz)
						for (int vx = vxStart; vx < vxEnd; ++vx)
						{
							const float h = heights[vz * W + vx];
							if (h < minH) minH = h;
							if (h > maxH) maxH = h;
						}
					if (minH > maxH) { minH = maxH = 0.0f; }

					const float srcMinY = originY + vzStart * cellSizeZ;  // 3ds Max Y（世界 Z の元）
					const float srcMaxY = originY + (vzEnd - 1) * cellSizeZ;

					const Vector3 worldMin(
						originX + vxStart * cellSizeX,
						minH + m_config.yOffset,
						-srcMaxY          // world_z = -src_y
					);
					const Vector3 worldMax(
						originX + (vxEnd - 1) * cellSizeX,
						maxH + m_config.yOffset,
						-srcMinY
					);
					m_chunkAABBs[chunkIdx].Init(worldMax, worldMin);

					// チャンクメッシュ生成
					char key[64];
					snprintf(key, sizeof(key), "%s_%d", TERRAIN_CHUNK_KEY_PREFIX, chunkIdx);
					m_chunkTkmFiles[chunkIdx] = buildTkm(
						vxStart, vxEnd, vzStart, vzEnd,
						cxStart, cxEnd, czStart, czEnd,
						key
					);
				}
			}
		}


		void TerrainObject::InitRenderer()
		{
			// BaseColor
			m_splatmap.InitFromDDSFile(SPLATMAP_PATH);
			m_terrainTextures[0].InitFromDDSFile(TEX_PATH_SNOW);
			m_terrainTextures[1].InitFromDDSFile(TEX_PATH_GRASS);
			m_terrainTextures[2].InitFromDDSFile(TEX_PATH_ROCK);

			// PBR テクスチャ（Normal / Roughness）
			m_snowNormal.    InitFromDDSFile(TEX_PATH_SNOW_NORMAL);
			m_snowRoughness. InitFromDDSFile(TEX_PATH_SNOW_ROUGHNESS);
			m_grassNormal.   InitFromDDSFile(TEX_PATH_GRASS_NORMAL);
			m_grassRoughness.InitFromDDSFile(TEX_PATH_GRASS_ROUGHNESS);
			m_rockNormal.    InitFromDDSFile(TEX_PATH_ROCK_NORMAL);
			m_rockRoughness. InitFromDDSFile(TEX_PATH_ROCK_ROUGHNESS);

			// ModelInitData の共通パラメータをセットするヘルパー
			auto buildInitData = [&](const char* tkmKey) -> ModelInitData
			{
				ModelInitData initData;
				initData.m_tkmFilePath              = tkmKey;
				initData.m_fxFilePath               = "Assets/shader/Terrain.fx";
				initData.m_expandConstantBuffer     = &m_terrainCb;
				initData.m_expandConstantBufferSize = static_cast<int>(sizeof(TerrainCb));

				initData.m_expandShaderResoruceView[0]  = &m_splatmap;
				initData.m_expandShaderResoruceView[1]  = &m_terrainTextures[0];  // snow
				initData.m_expandShaderResoruceView[2]  = &m_terrainTextures[1];  // grass
				initData.m_expandShaderResoruceView[3]  = &m_terrainTextures[2];  // rock
				// [4] は t14 に対応するが未使用のため nullptr のまま
				initData.m_expandShaderResoruceView[5]  = &m_snowNormal;
				initData.m_expandShaderResoruceView[6]  = &m_snowRoughness;
				initData.m_expandShaderResoruceView[7]  = &m_grassNormal;
				initData.m_expandShaderResoruceView[8]  = &m_grassRoughness;
				initData.m_expandShaderResoruceView[9]  = &m_rockNormal;
				initData.m_expandShaderResoruceView[10] = &m_rockRoughness;
				return initData;
			};

			// === 物理コリジョン用 ModelRender（描画には使わない） ===
			m_modelRender.SetGBufferFxFilePath("Assets/shader/Terrain.fx");
			m_modelRender.InitFromLoaded(buildInitData(TERRAIN_TKM_KEY));
			m_modelRender.SetPBRParam(m_config.pbrParam);
			m_modelRender.SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_modelRender.Update();
			m_physicalBody.CreateFromModel(
				m_modelRender.GetModel(),
				m_modelRender.GetModel().GetWorldMatrix(),
				nsBeastEngine::nsCollision::CollisionAttribute::Ground
			);

			// === チャンク別 ModelRender（フラスタムカリングで描画） ===
			const int numChunks = static_cast<int>(m_chunkTkmFiles.size());
			m_chunkRenders.resize(numChunks);

			for (int i = 0; i < numChunks; ++i)
			{
				if (m_chunkTkmFiles[i] == nullptr) continue;  // minHeight で全クワッドが除外されたチャンク

				char key[64];
				snprintf(key, sizeof(key), "%s_%d", TERRAIN_CHUNK_KEY_PREFIX, i);

				m_chunkRenders[i] = std::make_unique<nsBeastEngine::ModelRender>();
				m_chunkRenders[i]->SetGBufferFxFilePath("Assets/shader/Terrain.fx");
				m_chunkRenders[i]->InitFromLoaded(buildInitData(key));
				m_chunkRenders[i]->SetPBRParam(m_config.pbrParam);
				m_chunkRenders[i]->SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
				m_chunkRenders[i]->Update();
			}
		}
	}
}
