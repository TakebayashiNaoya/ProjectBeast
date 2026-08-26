/**
 * @file TerrainObject.cpp
 * @brief ハイトマップから生成する地形オブジェクト
 */
#include "stdafx.h"
#include "DirectXTex/DirectXTex.h"
#include "TerrainObject.h"


namespace
{
	/** ハイトマップ */
	static const wchar_t* HEIGHTMAP_PATH = L"Assets/modelData/stage/Terrain/TutorialStageHeightMap.dds";
	/** スプラットマップ */
	static const wchar_t* SPLATMAP_PATH = L"Assets/modelData/stage/Terrain/TutorialStageSplatMap.dds";

	/** エンジン内バンクに登録する際のキー（ファイルパスの代わりに使う合成キー） */
	static const char* TERRAIN_TKM_KEY = "terrain_generated";
	/** チャンク描画用メッシュのバンクキープレフィックス（"terrain_chunk_0"〜） */
	static const char* TERRAIN_CHUNK_KEY_PREFIX = "terrain_chunk";

	/** 地形テクスチャ（スプラットマップの R=snow, G=grass, B=rock） */
	static const wchar_t* TEX_PATH_SNOW = L"Assets/modelData/stage/Terrain/snow.DDS";
	static const wchar_t* TEX_PATH_GRASS = L"Assets/modelData/stage/Terrain/grass.DDS";
	static const wchar_t* TEX_PATH_ROCK = L"Assets/modelData/stage/Terrain/rock.DDS";

	/** PBR テクスチャ */
	static const wchar_t* TEX_PATH_SNOW_NORMAL = L"Assets/modelData/stage/Terrain/snow_Normal.DDS";
	static const wchar_t* TEX_PATH_SNOW_ROUGHNESS = L"Assets/modelData/stage/Terrain/snow_Roughness.DDS";
	static const wchar_t* TEX_PATH_GRASS_NORMAL = L"Assets/modelData/stage/Terrain/grass_Normal.DDS";
	static const wchar_t* TEX_PATH_GRASS_ROUGHNESS = L"Assets/modelData/stage/Terrain/grass_Roughness.DDS";
	static const wchar_t* TEX_PATH_ROCK_NORMAL = L"Assets/modelData/stage/Terrain/rock_Normal.DDS";
	static const wchar_t* TEX_PATH_ROCK_ROUGHNESS = L"Assets/modelData/stage/Terrain/rock_Roughness.DDS";
}


namespace app
{
	namespace actor
	{
		namespace
		{
			/**
			 * @brief ハイトマップ画素 (px, pz) → ワールド高さ値（境界外はクランプ）
			 *
			 * @param pixels      ハイトマップ画素配列（uint16_t）
			 * @param W           ハイトマップ横幅（画素数）
			 * @param H		      ハイトマップ縦幅（画素数）
			 * @param heightScale ハイトマップの最大高さ（ワールド単位）
			 * @param px		  ハイトマップ画素 X 座標
			 * @param pz		  ハイトマップ画素 Z 座標
			 *
			 * @return ワールド高さ値（ワールド単位）
			 */
			float SampleHeight(
				const std::vector<uint16_t>& pixels,
				int W,
				int H,
				float heightScale,
				int px,
				int pz
			)
			{
				px = max(0, min(W - 1, px));
				pz = max(0, min(H - 1, pz));
				return pixels[pz * W + px] / 65535.0f * heightScale;
			}


			/**
			 * @brief チャンク担当範囲の頂点バッファを生成する
			 *
			 * @param heights ハイトマップの高さ値配列（ワールド単位）
			 * @param hmPixels  ハイトマップの画素配列（uint16_t）
			 * @param W         ハイトマップ横幅（画素数）
			 * @param H		    ハイトマップ縦幅（画素数）
			 * @param cellSizeX ワールド座標系でのセル幅（X 方向）
			 * @param cellSizeZ ワールド座標系でのセル幅（Z 方向）
			 * @param originX   ワールド座標系での原点 X 座標
			 * @param originY   ワールド座標系での原点 Y 座標
			 * @param cfg       地形設定
			 * @param vxStart   チャンク担当範囲の頂点 X 開始インデックス
			 * @param vxEnd     チャンク担当範囲の頂点 X 終了インデックス（非包含）
			 * @param vzStart   チャンク担当範囲の頂点 Z 開始インデックス
			 * @param vzEnd     チャンク担当範囲の頂点 Z 終了インデックス（非包含）
			 *
			 * @return 頂点バッファ配列
			 */
			std::vector<TkmFile::SVertex> BuildChunkVertices(
				const std::vector<float>& heights,
				const std::vector<uint16_t>& hmPixels,
				int W,
				int H,
				float cellSizeX,
				float cellSizeZ,
				float originX,
				float originY,
				const TerrainObject::TerrainConfig& cfg,
				int vxStart,
				int vxEnd,
				int vzStart,
				int vzEnd
			)
			{
				const int localW = vxEnd - vxStart;  // チャンク内の頂点横方向数
				std::vector<TkmFile::SVertex> vertices;
				vertices.reserve(localW * (vzEnd - vzStart));

				for (int vz = vzStart; vz < vzEnd; ++vz)
				{
					for (int vx = vxStart; vx < vxEnd; ++vx)
					{
						const float h = heights[vz * W + vx];                                       // この頂点の高さ
						const float hR = SampleHeight(hmPixels, W, H, cfg.heightScale, vx + 1, vz);  // 右隣 (Right)
						const float hL = SampleHeight(hmPixels, W, H, cfg.heightScale, vx - 1, vz);  // 左隣 (Left)
						const float hU = SampleHeight(hmPixels, W, H, cfg.heightScale, vx, vz + 1);  // 奥隣 (Up/Forward)
						const float hD = SampleHeight(hmPixels, W, H, cfg.heightScale, vx, vz - 1);  // 手前 (Down/Back)

						// 中心差分法で法線を計算（Z-up 座標系）
						// X 方向接線: (2Cx, 0, hR-hL)、Y 方向接線: (0, 2Cz, hU-hD)
						Vector3 dx = { 2.0f * cellSizeX, 0.0f, hR - hL };
						Vector3 dy = { 0.0f, 2.0f * cellSizeZ, hU - hD };
						Vector3 normal = Cross(dx, dy);
						normal.Normalize();

						TkmFile::SVertex v;
						v.pos = Vector3(originX + vx * cellSizeX, originY + vz * cellSizeZ, h + cfg.yOffset);
						v.normal = normal;
						v.tangent = Vector3(1.0f, 0.0f, 0.0f);
						v.binormal = Vector3(0.0f, 1.0f, 0.0f);
						v.uv = Vector2(vx * cfg.uvTile, vz * cfg.uvTile);
						v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
						v.skinWeights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
						vertices.push_back(v);
					}
				}
				return vertices;
			}


			/**
			 * @brief チャンク担当範囲のインデックスバッファを生成する
			 *
			 * @param heights   ハイトマップの高さ値配列（ワールド単位）
			 * @param W         ハイトマップ横幅（画素数）
			 * @param minHeight この高さ未満の頂点を含むクワッドはポリゴンを生成しない（ワールド単位）
			 * @param cxStart   チャンク担当範囲のセル X 開始インデックス
			 * @param cxEnd     チャンク担当範囲のセル X 終了インデックス（非包含）
			 * @param czStart   チャンク担当範囲のセル Z 開始インデックス
			 * @param czEnd     チャンク担当範囲のセル Z 終了インデックス（非包含）
			 * @param vxStart   チャンク担当範囲の頂点 X 開始インデックス
			 * @param vzStart   チャンク担当範囲の頂点 Z 開始インデックス
			 * @param localW    チャンク内の頂点横方向数
			 *
			 * @return インデックスバッファ配列
			 */
			std::vector<uint32_t> BuildChunkIndices(
				const std::vector<float>& heights,
				int W,
				float minHeight,
				int cxStart,
				int cxEnd,
				int czStart,
				int czEnd,
				int vxStart,
				int vzStart,
				int localW
			)
			{
				std::vector<uint32_t> indices;
				indices.reserve((cxEnd - cxStart) * (czEnd - czStart) * 6);

				for (int cz = czStart; cz < czEnd; ++cz)
				{
					for (int cx = cxStart; cx < cxEnd; ++cx)
					{
						// 全頂点が minHeight 未満のクワッドはスキップ（海面下のメッシュを除去）
						if (minHeight > 0.0f)
						{
							// クワッドを構成する 4 頂点の高さ
							//   h0(cz,cx)    h1(cz,cx+1)
							//   h2(cz+1,cx)  h3(cz+1,cx+1)
							const float h0 = heights[cz * W + cx];
							const float h1 = heights[cz * W + (cx + 1)];
							const float h2 = heights[(cz + 1) * W + cx];
							const float h3 = heights[(cz + 1) * W + (cx + 1)];
							if (h0 < minHeight && h1 < minHeight && h2 < minHeight && h3 < minHeight)
								continue;
						}

						// チャンク内ローカル座標（vxStart/vzStart 基点）
						const uint32_t lz = static_cast<uint32_t>(cz - vzStart);
						const uint32_t lx = static_cast<uint32_t>(cx - vxStart);

						// クワッドの 4 頂点インデックス（頂点バッファ内の位置）
						//   i0---i1
						//   |  \ |
						//   i2---i3
						const uint32_t i0 = lz * localW + lx;        // 左上
						const uint32_t i1 = i0 + 1;                   // 右上
						const uint32_t i2 = (lz + 1) * localW + lx;  // 左下
						const uint32_t i3 = i2 + 1;                   // 右下

						// 三角形 1: i0-i1-i2、三角形 2: i1-i3-i2
						indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
						indices.push_back(i1); indices.push_back(i3); indices.push_back(i2);
					}
				}
				return indices;
			}


			/**
			 * @brief チャンク担当範囲の頂点・インデックスバッファから TkmFile を組み立ててバンクに登録する
			 *
			 * @param vertices チャンク担当範囲の頂点バッファ
			 * @param indices  チャンク担当範囲のインデックスバッファ
			 * @param bankKey  エンジン内バンクに登録する際のキー
			 *
			 * @return 登録した TkmFile（登録失敗時は nullptr）
			 */
			nsK2EngineLow::TkmFile* AssembleChunkTkm(
				std::vector<TkmFile::SVertex>&& vertices,
				std::vector<uint32_t>&& indices,
				const char* bankKey
			)
			{
				if (indices.empty()) return nullptr;  // minHeight で全クワッドが除外された場合

				TkmFile::SMaterial mat = {};

				TkmFile::SIndexBuffer32 ib;
				ib.indices = std::move(indices);

				TkmFile::SMesh mesh;
				mesh.isFlatShading = false;
				mesh.materials.push_back(mat);
				mesh.vertexBuffer = std::move(vertices);
				mesh.indexBuffer32Array.push_back(std::move(ib));

				std::vector<TkmFile::SMesh> meshes;
				meshes.push_back(std::move(mesh));

				auto* tkm = new TkmFile();
				tkm->Build(std::move(meshes));
				g_engine->ReplaceTkmFileInBank(bankKey, tkm);
				return tkm;
			}
		}// namespace


		TerrainObject::~TerrainObject()
		{
			for (auto& render : m_chunkRenders)
			{
				if (render)
					nsBeastEngine::OcclusionDitherManager::Get().Unregister(render.get());
			}
		}


		void TerrainObject::Init(const TerrainConfig& config)
		{
			// 一括版（デバッグのホットリロード用）。通常ロードは InitStep() を毎フレーム呼んで時分割する
			while (!InitStep(config)) {}
		}


		bool TerrainObject::InitStep(const TerrainConfig& config)
		{
			if (m_isInited) return true;

			// 各ステップを1フレーム約50ms以下に収める。まとめて実行すると
			// 1.5秒前後フレームが止まり、ローディングアイコンが凍って見える
			switch (m_initStep)
			{
			case 0:
				m_config = config;
				LoadHeightmap();
				LoadSplatmapCpu();
				m_initStep++;
				break;

			case 1:
				GenerateMesh();
				m_initStep++;
				break;

			case 2:
				if (InitRendererTextureStep()) m_initStep++;
				break;

			case 3:
				InitRendererPhysics();
				m_initStep++;
				break;

			case 4:
				if (InitRendererChunkStep()) m_initStep++;
				break;

			default:
				StartWrapper();
				m_isInited = true;
				break;
			}
			return m_isInited;
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

			// ここではカリングせず、全チャンクを描画リストへ登録する。
			// 以前はカメラの視錐台で弾いていたが、そうすると画面外のチャンクが
			// シャドウマップのキャスターからも外れてしまい、カメラを回すたびに
			// 地形の影が消えたり現れたりしていた。
			// 描画側のカリングは RenderingEngine::RenderToGBuffer() が、
			// 影側のカリングは ShadowMap がそれぞれの範囲で行う。
			const int numChunks = static_cast<int>(m_chunkRenders.size());
			for (int i = 0; i < numChunks; i++)
			{
				if (!m_chunkRenders[i]) continue;
				m_chunkRenders[i]->Draw(rc);
			}
		}


		void TerrainObject::LoadHeightmap()
		{
			// DirectXTex で DDS をロード（BC1〜BC7, BC6H を含む全圧縮フォーマット対応）
			DirectX::ScratchImage image;
			HRESULT hr = DirectX::LoadFromDDSFile(m_config.heightmapPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);

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
			const int    srcW = static_cast<int>(img->width);
			const int    srcH = static_cast<int>(img->height);
			const int    rowPitch = static_cast<int>(img->rowPitch) / sizeof(float);
			const float* pixels = reinterpret_cast<const float*>(img->pixels);

			m_heightmap.width = srcW / m_config.subsample;
			m_heightmap.height = srcH / m_config.subsample;
			m_heightmap.pixels.assign(m_heightmap.width * m_heightmap.height, 0);

			for (int dstZ = 0; dstZ < m_heightmap.height; ++dstZ) {
				for (int dstX = 0; dstX < m_heightmap.width; ++dstX) {
					const float f = pixels[(dstZ * m_config.subsample) * rowPitch + (dstX * m_config.subsample)];
					m_heightmap.pixels[dstZ * m_heightmap.width + dstX] = static_cast<uint16_t>(std::clamp(f, 0.0f, 1.0f) * 65535.0f);
				}
			}
		}


		void TerrainObject::GenerateMesh()
		{
			// ハイトマップの幅と高さ（画素数）
			const int W = m_heightmap.width;
			const int H = m_heightmap.height;
			// 頂点間隔を算出（3ds Max Z-up 座標系）
			const float cellSizeX = (W > 1) ? m_config.totalWidth / float(W - 1) : 1.0f;
			const float cellSizeZ = (H > 1) ? m_config.totalDepth / float(H - 1) : 1.0f;
			const float originX = -m_config.totalWidth * 0.5f;
			const float originY = -m_config.totalDepth * 0.5f;  // 3ds Max の奥行き軸
			// 地形定数バッファを設定
			m_terrainCb.halfWidth = m_config.totalWidth * 0.5f;
			m_terrainCb.halfDepth = m_config.totalDepth * 0.5f;
			m_terrainCb.albedoScale = m_config.albedoScale;

			// 高さキャッシュ（全頂点の高さを事前計算して再利用）
			std::vector<float> heights(W * H);
			for (int z = 0; z < H; ++z) {
				for (int x = 0; x < W; ++x) {
					heights[z * W + x] = SampleHeight(m_heightmap.pixels, W, H, m_config.heightScale, x, z);
				}
			}

			// 歩行可否グリッド（簡易ナビゲーション）を高さキャッシュから構築する
			m_navGrid.Build(heights, W, H, m_config.totalWidth, m_config.totalDepth, m_config.yOffset);

			// 物理用フルメッシュ（描画には使わず当たり判定のみ）
			{
				// チャンク担当範囲の頂点バッファを生成する（全頂点を使う）
				auto verts = BuildChunkVertices(heights, m_heightmap.pixels, W, H, cellSizeX, cellSizeZ, originX, originY, m_config, 0, W, 0, H);
				// チャンク担当範囲のインデックスバッファを生成する（全セルを使う）
				auto indices = BuildChunkIndices(heights, W, m_config.minHeight, 0, W - 1, 0, H - 1, 0, 0, W);
				// TkmFile を組み立ててバンクに登録する
				m_tkmFile = AssembleChunkTkm(std::move(verts), std::move(indices), TERRAIN_TKM_KEY);
			}

			// チャンク別描画メッシュ
			{
				//   頂点インデックス (vx, vz) : ハイトマップの画素単位。W×H 個存在する。
				//   セルインデックス  (cx, cz) : 隣り合う 4 頂点が作るクワッド単位。(W-1)×(H-1) 個存在する。
				//   チャンク          : フラスタムカリング用に地形を chunkDiv×chunkDiv に分割した区画。
				const int chunkDiv = m_config.chunkDivision;  // 縦横それぞれの分割数
				const int numCellsX = W - 1;                   // X 方向の総セル数
				const int numCellsZ = H - 1;                   // Z 方向の総セル数
				// チャンクあたりのセル数（切り上げ除算：端のチャンクだけ小さくなることを許容）
				const int cpchX = (numCellsX + chunkDiv - 1) / chunkDiv;  // cells per chunk X
				const int cpchZ = (numCellsZ + chunkDiv - 1) / chunkDiv;  // cells per chunk Z
				const int numChunks = chunkDiv * chunkDiv;                     // チャンク総数

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
						const int cxEnd = min(cxStart + cpchX, numCellsX);
						const int czEnd = min(czStart + cpchZ, numCellsZ);

						// 頂点範囲（セル範囲より各辺1大きい：セルの右端・下端の頂点も必要なため）
						const int vxStart = cxStart;
						const int vzStart = czStart;
						const int vxEnd = min(cxEnd + 1, W);
						const int vzEnd = min(czEnd + 1, H);

						// ワールド空間 AABB を計算する
						// BeastModel は MakeRotationX(-PI/2) で 3ds Max Z-up → DX Y-up に変換する。
						// 変換後: world_x=src_x, world_y=src_z(高さ), world_z=-src_y(奥行き)
						float minH = FLT_MAX, maxH = -FLT_MAX;
						for (int vz = vzStart; vz < vzEnd; ++vz)
						{
							for (int vx = vxStart; vx < vxEnd; ++vx)
							{
								const float h = heights[vz * W + vx];
								if (h < minH) minH = h;
								if (h > maxH) maxH = h;
							}
						}
						if (minH > maxH) { minH = maxH = 0.0f; }

						const float srcMinY = originY + vzStart * cellSizeZ;      // 3ds Max Y（世界 Z の元）
						const float srcMaxY = originY + (vzEnd - 1) * cellSizeZ;

						const Vector3 worldMin(originX + vxStart * cellSizeX, minH + m_config.yOffset, -srcMaxY);
						const Vector3 worldMax(originX + (vxEnd - 1) * cellSizeX, maxH + m_config.yOffset, -srcMinY);
						m_chunkAABBs[chunkIdx].Init(worldMax, worldMin);

						// チャンクメッシュ生成
						const int localW = vxEnd - vxStart;  // チャンク内の頂点横方向数
						auto verts = BuildChunkVertices(heights, m_heightmap.pixels, W, H, cellSizeX, cellSizeZ, originX, originY, m_config, vxStart, vxEnd, vzStart, vzEnd);
						auto indices = BuildChunkIndices(heights, W, m_config.minHeight, cxStart, cxEnd, czStart, czEnd, vxStart, vzStart, localW);

						char key[64];
						snprintf(key, sizeof(key), "%s_%d", TERRAIN_CHUNK_KEY_PREFIX, chunkIdx);
						m_chunkTkmFiles[chunkIdx] = AssembleChunkTkm(std::move(verts), std::move(indices), key);
					}
				}
			}
		}


		void TerrainObject::SetupModelInitData(nsK2EngineLow::ModelInitData& initData, const char* tkmKey)
		{
			initData.m_tkmFilePath = tkmKey;
			initData.m_fxFilePath = "Assets/shader/Terrain.fx";
			initData.m_expandConstantBuffer = &m_terrainCb;
			initData.m_expandConstantBufferSize = static_cast<int>(sizeof(TerrainCb));

			initData.m_expandShaderResoruceView[0] = &m_splatmap;
			initData.m_expandShaderResoruceView[1] = &m_terrainTextures[0];  // snow
			initData.m_expandShaderResoruceView[2] = &m_terrainTextures[1];  // grass
			initData.m_expandShaderResoruceView[3] = &m_terrainTextures[2];  // rock
			// [4] は t14 に対応するが未使用のため nullptr のまま
			initData.m_expandShaderResoruceView[5] = &m_snowNormal;
			initData.m_expandShaderResoruceView[6] = &m_snowRoughness;
			initData.m_expandShaderResoruceView[7] = &m_grassNormal;
			initData.m_expandShaderResoruceView[8] = &m_grassRoughness;
			initData.m_expandShaderResoruceView[9] = &m_rockNormal;
			initData.m_expandShaderResoruceView[10] = &m_rockRoughness;
		}


		bool TerrainObject::InitRendererTextureStep()
		{
			// 1回の呼び出しで1枚だけ読む（1枚約40ms。11枚まとめると0.4秒フレームが止まる）
			nsK2EngineLow::Texture* textures[] = {
				&m_splatmap, &m_heightmapTextureGpu,
				&m_terrainTextures[0], &m_terrainTextures[1], &m_terrainTextures[2],
				&m_snowNormal, &m_snowRoughness,
				&m_grassNormal, &m_grassRoughness,
				&m_rockNormal, &m_rockRoughness,
			};
			const wchar_t* paths[] = {
				m_config.splatmapPath.c_str(), m_config.heightmapPath.c_str(),
				TEX_PATH_SNOW, TEX_PATH_GRASS, TEX_PATH_ROCK,
				TEX_PATH_SNOW_NORMAL, TEX_PATH_SNOW_ROUGHNESS,
				TEX_PATH_GRASS_NORMAL, TEX_PATH_GRASS_ROUGHNESS,
				TEX_PATH_ROCK_NORMAL, TEX_PATH_ROCK_ROUGHNESS,
			};
			static_assert(_countof(textures) == _countof(paths), "テクスチャとパスの数を揃えること");

			if (m_textureInitIndex < _countof(textures))
			{
				textures[m_textureInitIndex]->InitFromDDSFile(paths[m_textureInitIndex]);
				m_textureInitIndex++;
			}
			return m_textureInitIndex >= _countof(textures);
		}


		void TerrainObject::InitRendererPhysics()
		{
			// 物理コリジョン用 ModelRender（描画には使わない）
			nsK2EngineLow::ModelInitData initData;
			SetupModelInitData(initData, TERRAIN_TKM_KEY);
			m_modelRender.SetGBufferFxFilePath("Assets/shader/Terrain.fx");
			m_modelRender.InitFromLoaded(initData);
			m_modelRender.SetPBRParam(m_config.pbrParam);
			m_modelRender.SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_modelRender.Update();
			m_physicalBody.CreateFromModel(
				m_modelRender.GetModel(),
				m_modelRender.GetModel().GetWorldMatrix(),
				nsBeastEngine::nsCollision::CollisionAttribute::Ground
			);
		}


		bool TerrainObject::InitRendererChunkStep()
		{
			// チャンク別 ModelRender（フラスタムカリングで描画）。
			// 全チャンク一括だと0.6秒前後フレームが止まるため、時間予算内で数個ずつ進める
			constexpr double CHUNK_INIT_BUDGET_MS = 25.0;

			const int numChunks = static_cast<int>(m_chunkTkmFiles.size());
			if (m_chunkRenders.empty() && numChunks > 0)
			{
				m_chunkRenders.resize(numChunks);
			}

			LARGE_INTEGER freq, begin, now;
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&begin);

			while (m_chunkInitIndex < numChunks)
			{
				const int i = m_chunkInitIndex++;
				if (m_chunkTkmFiles[i] == nullptr) continue;  // minHeight で全クワッドが除外されたチャンク

				char key[64];
				snprintf(key, sizeof(key), "%s_%d", TERRAIN_CHUNK_KEY_PREFIX, i);

				nsK2EngineLow::ModelInitData initData;
				SetupModelInitData(initData, key);
				m_chunkRenders[i] = std::make_unique<nsBeastEngine::ModelRender>();
				m_chunkRenders[i]->SetGBufferFxFilePath("Assets/shader/Terrain.fx");
				m_chunkRenders[i]->InitFromLoaded(initData);
				m_chunkRenders[i]->SetPBRParam(m_config.pbrParam);
				m_chunkRenders[i]->SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
				m_chunkRenders[i]->Update();
				nsBeastEngine::OcclusionDitherManager::Get().Register(m_chunkRenders[i].get());

				QueryPerformanceCounter(&now);
				const double elapsedMs = 1000.0 * (now.QuadPart - begin.QuadPart) / freq.QuadPart;
				if (elapsedMs > CHUNK_INIT_BUDGET_MS) break;
			}
			return m_chunkInitIndex >= numChunks;
		}


		void TerrainObject::LoadSplatmapCpu()
		{
			DirectX::ScratchImage image;
			HRESULT hr = DirectX::LoadFromDDSFile(m_config.splatmapPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
			if (FAILED(hr))
			{
				m_splatmapCpu.width = m_splatmapCpu.height = 0;
				return;
			}

			const DirectX::Image* img = image.GetImage(0, 0, 0);

			DirectX::ScratchImage decompressed;
			if (DirectX::IsCompressed(img->format))
			{
				if (FAILED(DirectX::Decompress(*img, DXGI_FORMAT_R8G8B8A8_UNORM, decompressed)))
				{
					m_splatmapCpu.width = m_splatmapCpu.height = 0;
					return;
				}
				img = decompressed.GetImage(0, 0, 0);
			}

			DirectX::ScratchImage converted;
			if (img->format != DXGI_FORMAT_R8G8B8A8_UNORM)
			{
				if (FAILED(DirectX::Convert(
					*img, DXGI_FORMAT_R8G8B8A8_UNORM,
					DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT,
					converted)))
				{
					m_splatmapCpu.width = m_splatmapCpu.height = 0;
					return;
				}
				img = converted.GetImage(0, 0, 0);
			}

			const int srcW = static_cast<int>(img->width);
			const int srcH = static_cast<int>(img->height);
			const int rowPitch = static_cast<int>(img->rowPitch);

			m_splatmapCpu.width = srcW;
			m_splatmapCpu.height = srcH;
			m_splatmapCpu.pixels.assign(srcW * srcH * 4, 0);

			for (int y = 0; y < srcH; ++y)
			{
				memcpy(&m_splatmapCpu.pixels[y * srcW * 4], img->pixels + y * rowPitch, srcW * 4);
			}
		}


		TerrainObject::SurfaceType TerrainObject::GetSurfaceTypeAt(const Vector3& worldPos) const
		{
			if (m_splatmapCpu.pixels.empty()) return SurfaceType::Snow;

			// ★GenerateMesh()の座標変換に合わせて逆算（worldZが増えるほどvzは減る点に注意）
			const float u = (worldPos.x + m_terrainCb.halfWidth) / (m_terrainCb.halfWidth * 2.0f);
			const float v = (m_terrainCb.halfDepth - worldPos.z) / (m_terrainCb.halfDepth * 2.0f);

			int px = static_cast<int>(u * m_splatmapCpu.width);
			int pz = static_cast<int>(v * m_splatmapCpu.height);
			px = max(0, min(m_splatmapCpu.width - 1, px));
			pz = max(0, min(m_splatmapCpu.height - 1, pz));

			const int idx = (pz * m_splatmapCpu.width + px) * 4;
			const uint8_t r = m_splatmapCpu.pixels[idx + 0]; // 雪
			const uint8_t g = m_splatmapCpu.pixels[idx + 1]; // 草
			const uint8_t b = m_splatmapCpu.pixels[idx + 2]; // 岩

			if (g >= r && g >= b) return SurfaceType::Grass;
			if (b >= r && b >= g) return SurfaceType::Rock;
			return SurfaceType::Snow;
		}


		float TerrainObject::SampleHeightBilinear(float fx, float fz) const
		{
			const int W = m_heightmap.width;
			const int H = m_heightmap.height;
			if (W <= 0 || H <= 0) return 0.0f;

			fx = max(0.0f, min(float(W - 1), fx));
			fz = max(0.0f, min(float(H - 1), fz));

			const int x0 = static_cast<int>(fx);
			const int z0 = static_cast<int>(fz);
			const int x1 = min(W - 1, x0 + 1);
			const int z1 = min(H - 1, z0 + 1);

			const float tx = fx - x0;
			const float tz = fz - z0;

			auto sample = [&](int x, int z) -> float
				{
					return m_heightmap.pixels[z * W + x] / 65535.0f * m_config.heightScale;
				};

			const float h00 = sample(x0, z0);
			const float h10 = sample(x1, z0);
			const float h01 = sample(x0, z1);
			const float h11 = sample(x1, z1);

			const float h0 = h00 + (h10 - h00) * tx;
			const float h1 = h01 + (h11 - h01) * tx;
			return h0 + (h1 - h0) * tz;
		}


		float TerrainObject::GetHeightAt(const Vector3& worldPos) const
		{
			if (m_heightmap.pixels.empty()) return 0.0f;

			const int W = m_heightmap.width;
			const int H = m_heightmap.height;

			// ワールド座標 → ハイトマップ画素座標（GenerateMesh()の座標変換の逆算）
			const float fx = (worldPos.x + m_terrainCb.halfWidth) / (m_terrainCb.halfWidth * 2.0f) * (W - 1);
			const float fz = (m_terrainCb.halfDepth - worldPos.z) / (m_terrainCb.halfDepth * 2.0f) * (H - 1);

			return SampleHeightBilinear(fx, fz) + m_config.yOffset;
		}
	}
}
