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

	/** エンジン内バンクに登録する際のキー（ファイルパスの代わりに使う合成キー） */
	static const char* TERRAIN_TKM_KEY = "terrain_generated";

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
		}


		void TerrainObject::Render(RenderContext& rc)
		{
			if (!m_isInited) return;
			m_modelRender.Draw(rc);
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
			// エンジンは BeastModel::CalcWorldMatrix で MakeRotationX(-PI/2) を適用し
			// Z-up → DirectX Y-up に変換するため、頂点は Z-up で生成する必要がある
			const float cellSizeX = (W > 1) ? m_config.totalWidth  / float(W - 1) : 1.0f;
			const float cellSizeZ = (H > 1) ? m_config.totalDepth  / float(H - 1) : 1.0f;
			const float originX   = -m_config.totalWidth  * 0.5f;
			const float originY   = -m_config.totalDepth  * 0.5f;  // 3ds Max の奥行き軸（Y）

			// TerrainCb（b1）に地形パラメータを設定
			m_terrainCb.halfWidth   = m_config.totalWidth  * 0.5f;
			m_terrainCb.halfDepth   = m_config.totalDepth  * 0.5f;
			m_terrainCb.albedoScale = m_config.albedoScale;

			// getH はループ外で定義（頂点生成とインデックス生成の両方で使う）
			auto getH = [&](int px, int pz) -> float
			{
				px = max(0, min(W - 1, px));
				pz = max(0, min(H - 1, pz));
				return m_heightmap.pixels[pz * W + px] / 65535.0f * m_config.heightScale;
			};

			// 高さキャッシュ（インデックス生成時の minHeight チェックに使う）
			std::vector<float> heights(W * H);
			for (int z = 0; z < H; ++z) {
				for (int x = 0; x < W; ++x) {
					heights[z * W + x] = getH(x, z);
				}
			}

			// ------ 頂点生成 ------
			std::vector<TkmFile::SVertex> vertices;
			vertices.reserve(W * H);

			for (int z = 0; z < H; ++z)
			{
				for (int x = 0; x < W; ++x)
				{
					const float h  = heights[z * W + x];
					const float hR = getH(x + 1, z);
					const float hL = getH(x - 1, z);
					const float hU = getH(x, z + 1);
					const float hD = getH(x, z - 1);

					// 3ds Max Z-up 座標系での中心差分法線
					// X 接線: (2Cx, 0, hR-hL)、Y 接線: (0, 2Cz, hU-hD)
					// Cross(dx, dy) → 平坦地形で (0,0,4CxCz) → 正規化 → (0,0,1) Z-up 法線
					Vector3 dx = { 2.0f * cellSizeX, 0.0f, hR - hL };
					Vector3 dy = { 0.0f, 2.0f * cellSizeZ, hU - hD };
					Vector3 normal = Cross(dx, dy);
					normal.Normalize();

					TkmFile::SVertex v;
					// yOffset で地形全体を Y 方向にシフト（3ds Max Z-up の Z 軸 = DirectX の Y 軸）
					v.pos         = Vector3(originX + x * cellSizeX, originY + z * cellSizeZ, h + m_config.yOffset);
					v.normal      = normal;
					v.tangent     = Vector3(1.0f, 0.0f, 0.0f);
					v.binormal    = Vector3(0.0f, 1.0f, 0.0f);
					v.uv          = Vector2(x * m_config.uvTile, z * m_config.uvTile);
					v.indices[0]  = v.indices[1] = v.indices[2] = v.indices[3] = 0;
					v.skinWeights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
					vertices.push_back(v);
				}
			}

			// ------ インデックス生成（各セル 2 三角形） ------
			std::vector<uint32_t> indices;
			indices.reserve((W - 1) * (H - 1) * 6);

			for (int z = 0; z < H - 1; ++z)
			{
				for (int x = 0; x < W - 1; ++x)
				{
					// minHeight 未満の頂点を含むクワッドはスキップ（海面下のメッシュを除去）
					if (m_config.minHeight > 0.0f)
					{
						const float h0 = heights[ z      * W +  x     ];
						const float h1 = heights[ z      * W + (x + 1)];
						const float h2 = heights[(z + 1) * W +  x     ];
						const float h3 = heights[(z + 1) * W + (x + 1)];
						if (h0 < m_config.minHeight || h1 < m_config.minHeight || h2 < m_config.minHeight || h3 < m_config.minHeight)
							continue;
					}

					uint32_t i0 = z * W + x;
					uint32_t i1 = i0 + 1;
					uint32_t i2 = (z + 1) * W + x;
					uint32_t i3 = i2 + 1;

					indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
					indices.push_back(i1); indices.push_back(i3); indices.push_back(i2);
				}
			}

			// ------ TkmFile 構築 ------
			// マテリアルの LowTexture ポインタを nullptr にすると
			// Material::InitTexture がエンジン内のヌルテクスチャマップを自動使用する
			TkmFile::SMaterial mat = {};
			mat.uniqID         = 0;
			mat.albedoMap      = nullptr;
			mat.normalMap      = nullptr;
			mat.specularMap    = nullptr;
			mat.reflectionMap  = nullptr;
			mat.refractionMap  = nullptr;

			TkmFile::SIndexBuffer32 ib;
			ib.indices = std::move(indices);

			TkmFile::SMesh mesh;
			mesh.isFlatShading = false;
			mesh.materials.push_back(mat);
			mesh.vertexBuffer  = std::move(vertices);
			mesh.indexBuffer32Array.push_back(std::move(ib));

			std::vector<TkmFile::SMesh> meshes;
			meshes.push_back(std::move(mesh));

			// ヒープに確保してバンクに所有権を委譲する
			// TResourceBank は unique_ptr で管理するため、スタック変数のアドレスを渡すと
			// エンジン終了時に delete が呼ばれ heap corruption になる
			m_tkmFile = new TkmFile();
			m_tkmFile->Build(std::move(meshes));
			g_engine->RegistTkmFileToBank(TERRAIN_TKM_KEY, m_tkmFile);
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


			// GBuffer パスを Terrain.fx に変更（Init より前に呼ぶ必要がある）
			m_modelRender.SetGBufferFxFilePath("Assets/shader/Terrain.fx");

			ModelInitData initData;
			initData.m_tkmFilePath = TERRAIN_TKM_KEY;
			initData.m_fxFilePath  = "Assets/shader/Terrain.fx";

			// b1: TerrainCb（地形パラメータ）
			initData.m_expandConstantBuffer     = &m_terrainCb;
			initData.m_expandConstantBufferSize = static_cast<int>(sizeof(TerrainCb));

			// 拡張 SRV レジスタ対応表:
			//  [0]=t10 splatmap  [1]=t11 snow  [2]=t12 grass  [3]=t13 rock
			//  [4]=t14 (未使用)
			//  [5]=t15 snowNormal  [6]=t16 snowRoughness
			//  [7]=t17 grassNormal [8]=t18 grassRoughness
			//  [9]=t19 rockNormal  [10]=t20 rockRoughness
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

			m_modelRender.InitFromLoaded(initData);

			// 他モデルと同じ per-model PBR パラメータ制御（b2 PBRParamCb に反映される）
			m_modelRender.SetPBRParam(m_config.pbrParam);

			// ワールド行列を確定させてから当たり判定を生成
			m_modelRender.SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_modelRender.Update();
			m_physicalBody.CreateFromModel(
				m_modelRender.GetModel(),
				m_modelRender.GetModel().GetWorldMatrix(),
				nsBeastEngine::nsCollision::CollisionAttribute::Ground
			);
		}
	}
}
