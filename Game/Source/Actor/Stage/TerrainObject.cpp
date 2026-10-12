/**
 * @file TerrainObject.cpp
 * @brief ハイトマップから生成する地形オブジェクト
 */
#include "stdafx.h"
#include "DirectXTex/DirectXTex.h"
#include "TerrainObject.h"
#include <cstdarg>


namespace
{
	/** ハイトマップ */
	static const wchar_t* HEIGHTMAP_PATH = L"Assets/modelData/stage/Terrain/NormalStageHeightMap.dds";
	/** スプラットマップ */
	static const wchar_t* SPLATMAP_PATH = L"Assets/modelData/stage/Terrain/NormalStageSplatMap.dds";

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

	/**
	 * 高さの差（AABBのY幅）がこれより小さい平らな地形チャンクは影を落とさない
	 * @details 平らな面の影は自分自身にしか落ちず、シャドウマップに描いても見た目は変わらない。
	 *          地形チャンクは数が多く、影を描く数がそのままGPU時間になるため省く。
	 *          山や崖を含むチャンクは高さの差が大きいので、今まで通り影を落とす。
	 */
	static constexpr float FLAT_CHUNK_SHADOW_HEIGHT_RANGE = 30.0f;

	/**
	 * チャンクのLODごとの頂点の間隔（セル数）。[0]はフル解像度
	 * @details 遠くのチャンクは画面上で三角形が1ピクセルより小さくなるため、頂点を間引いたメッシュで描く。
	 *          描画用のメッシュだけを間引き、当たり判定（物理用フルメッシュ）・高さ取得・ナビグリッドには影響しない。
	 */
	static constexpr int CHUNK_LOD_STRIDES[] = { 1, 4, 8 };
	/**
	 * LODを切り替えるカメラからの距離（ワールド単位）。[i] 未満なら LOD i を使い、全て超えたら最も粗いLOD
	 * @details 画面上の三角形の大きさがだいたい十数ピクセルに揃うように決めた値
	 */
	static constexpr float CHUNK_LOD_DISTANCES[] = { 2000.0f, 4000.0f };
	/**
	 * 遠いカスケード（1番以降）の影に使うLOD
	 * @details 遠いカスケードは範囲が広くカメラの近くも含むが、近くの地形が受ける影はカスケード0から取る。
	 *          そのため遠いカスケードには、近くのチャンクも粗いメッシュで影を描けば足りる
	 */
	static constexpr int FAR_CASCADE_SHADOW_LOD = 2;

	/** 地形のベイク影で、影の境界をぼかす高さの幅（ワールド単位） */
	static constexpr float TERRAIN_SHADOW_SOFTNESS = 8.0f;
	/** 地形のベイク影で、影の判定を甘くする高さ（自分自身の影で縞が出るのを防ぐ） */
	static constexpr float TERRAIN_SHADOW_BIAS = 2.0f;
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
			 * @brief チャンクのLODメッシュ（頂点を間引いたメッシュ＋つなぎ目を隠すスカート）を生成する
			 * @details 頂点は stride セルおきに置く（チャンクの端は必ず含める）。
			 *          隣のチャンクとLODが違うと境目に隙間ができるため、チャンクの縁から下向きに
			 *          スカート（垂れ幕状のポリゴン）を出して隠す。スカートは両面に張る。
			 *
			 * @param heights     ハイトマップの高さ値配列（ワールド単位）
			 * @param hmPixels    ハイトマップの画素配列（uint16_t）
			 * @param W           ハイトマップ横幅（画素数）
			 * @param H           ハイトマップ縦幅（画素数）
			 * @param cellSizeX   ワールド座標系でのセル幅（X 方向）
			 * @param cellSizeZ   ワールド座標系でのセル幅（Z 方向）
			 * @param originX     ワールド座標系での原点 X 座標
			 * @param originY     ワールド座標系での原点 Y 座標
			 * @param cfg         地形設定
			 * @param cxStart     チャンク担当範囲のセル X 開始インデックス
			 * @param cxEnd       チャンク担当範囲のセル X 終了インデックス（非包含）
			 * @param czStart     チャンク担当範囲のセル Z 開始インデックス
			 * @param czEnd       チャンク担当範囲のセル Z 終了インデックス（非包含）
			 * @param stride      頂点を置く間隔（セル数。1でフル解像度）
			 * @param skirtDepth  スカートを下に伸ばす長さ（ワールド単位）
			 * @param outVertices 生成した頂点バッファ
			 * @param outIndices  生成したインデックスバッファ（ポリゴンが1つも無ければ空）
			 */
			void BuildChunkLodMesh(
				const std::vector<float>& heights,
				const std::vector<uint16_t>& hmPixels,
				int W,
				int H,
				float cellSizeX,
				float cellSizeZ,
				float originX,
				float originY,
				const TerrainObject::TerrainConfig& cfg,
				int cxStart,
				int cxEnd,
				int czStart,
				int czEnd,
				int stride,
				float skirtDepth,
				std::vector<TkmFile::SVertex>& outVertices,
				std::vector<uint32_t>& outIndices
			)
			{
				// 頂点を置く位置（ハイトマップの画素単位）。端は必ず含める
				const auto buildSamplePositions = [stride](const int start, const int end)
					{
						std::vector<int> positions;
						for (int v = start; v < end; v += stride) positions.push_back(v);
						positions.push_back(end);
						return positions;
					};
				const std::vector<int> xs = buildSamplePositions(cxStart, cxEnd);
				const std::vector<int> zs = buildSamplePositions(czStart, czEnd);
				const int numX = static_cast<int>(xs.size());
				const int numZ = static_cast<int>(zs.size());

				// 1頂点ぶんを作る（法線はフル解像度の隣の画素から求め、LODが変わっても陰影がなるべく変わらないようにする）
				const auto makeVertex = [&](const int vx, const int vz, const float yOffset)
					{
						const float h = heights[vz * W + vx];
						const float hR = SampleHeight(hmPixels, W, H, cfg.heightScale, vx + 1, vz);
						const float hL = SampleHeight(hmPixels, W, H, cfg.heightScale, vx - 1, vz);
						const float hU = SampleHeight(hmPixels, W, H, cfg.heightScale, vx, vz + 1);
						const float hD = SampleHeight(hmPixels, W, H, cfg.heightScale, vx, vz - 1);
						Vector3 dx = { 2.0f * cellSizeX, 0.0f, hR - hL };
						Vector3 dy = { 0.0f, 2.0f * cellSizeZ, hU - hD };
						Vector3 normal = Cross(dx, dy);
						normal.Normalize();

						TkmFile::SVertex v;
						v.pos = Vector3(originX + vx * cellSizeX, originY + vz * cellSizeZ, h + cfg.yOffset + yOffset);
						v.normal = normal;
						v.tangent = Vector3(1.0f, 0.0f, 0.0f);
						v.binormal = Vector3(0.0f, 1.0f, 0.0f);
						v.uv = Vector2(vx * cfg.uvTile, vz * cfg.uvTile);
						v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
						v.skinWeights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
						return v;
					};

				outVertices.clear();
				outIndices.clear();
				outVertices.reserve(numX * numZ + (numX + numZ) * 2);
				for (int iz = 0; iz < numZ; ++iz)
				{
					for (int ix = 0; ix < numX; ++ix)
					{
						outVertices.push_back(makeVertex(xs[ix], zs[iz], 0.0f));
					}
				}

				// クワッドを作るか（海面下の判定はフル解像度の全頂点で行い、粗いメッシュでも海岸に穴を開けない）
				std::vector<uint8_t> hasQuad((numX - 1) * (numZ - 1), 0);
				for (int iz = 0; iz + 1 < numZ; ++iz)
				{
					for (int ix = 0; ix + 1 < numX; ++ix)
					{
						bool isAboveMinHeight = (cfg.minHeight <= 0.0f);
						for (int vz = zs[iz]; vz <= zs[iz + 1] && !isAboveMinHeight; ++vz)
						{
							for (int vx = xs[ix]; vx <= xs[ix + 1]; ++vx)
							{
								if (heights[vz * W + vx] >= cfg.minHeight)
								{
									isAboveMinHeight = true;
									break;
								}
							}
						}
						if (!isAboveMinHeight) continue;
						hasQuad[iz * (numX - 1) + ix] = 1;

						//   i0---i1
						//   |  \ |
						//   i2---i3
						const uint32_t i0 = iz * numX + ix;
						const uint32_t i1 = i0 + 1;
						const uint32_t i2 = (iz + 1) * numX + ix;
						const uint32_t i3 = i2 + 1;
						outIndices.push_back(i0); outIndices.push_back(i1); outIndices.push_back(i2);
						outIndices.push_back(i1); outIndices.push_back(i3); outIndices.push_back(i2);
					}
				}
				if (outIndices.empty()) return;

				// スカート：縁の線分ごとに、隣接するクワッドがあれば下向きの両面ポリゴンを張る
				const auto addSkirt = [&](const uint32_t topA, const uint32_t topB)
					{
						const uint32_t bottomA = static_cast<uint32_t>(outVertices.size());
						TkmFile::SVertex va = outVertices[topA];
						va.pos.z -= skirtDepth;   // 3ds Max Z-up の高さ方向
						outVertices.push_back(va);
						const uint32_t bottomB = static_cast<uint32_t>(outVertices.size());
						TkmFile::SVertex vb = outVertices[topB];
						vb.pos.z -= skirtDepth;
						outVertices.push_back(vb);

						// 表
						outIndices.push_back(topA); outIndices.push_back(topB); outIndices.push_back(bottomA);
						outIndices.push_back(topB); outIndices.push_back(bottomB); outIndices.push_back(bottomA);
						// 裏
						outIndices.push_back(topA); outIndices.push_back(bottomA); outIndices.push_back(topB);
						outIndices.push_back(topB); outIndices.push_back(bottomA); outIndices.push_back(bottomB);
					};
				for (int ix = 0; ix + 1 < numX; ++ix)
				{
					// 手前の縁（iz = 0）と奥の縁（iz = numZ - 1）
					if (hasQuad[0 * (numX - 1) + ix]) addSkirt(ix, ix + 1);
					if (hasQuad[(numZ - 2) * (numX - 1) + ix]) addSkirt((numZ - 1) * numX + ix, (numZ - 1) * numX + ix + 1);
				}
				for (int iz = 0; iz + 1 < numZ; ++iz)
				{
					// 左の縁（ix = 0）と右の縁（ix = numX - 1）
					if (hasQuad[iz * (numX - 1) + 0]) addSkirt(iz * numX, (iz + 1) * numX);
					if (hasQuad[iz * (numX - 1) + (numX - 2)]) addSkirt(iz * numX + (numX - 1), (iz + 1) * numX + (numX - 1));
				}
			}


			/**
			 * @brief チャンクの縁で、最も粗いLODの直線とフル解像度の高さの最大のずれを求める
			 * @details 隣り合うチャンクのLODが違うときの隙間の大きさの目安。スカートの長さに使う。
			 */
			float CalcChunkEdgeDeviation(
				const std::vector<float>& heights,
				int W,
				int cxStart,
				int cxEnd,
				int czStart,
				int czEnd,
				int stride
			)
			{
				float maxDeviation = 0.0f;
				// 1本の縁（一定の間隔で並ぶ頂点）について、stride ごとの直線補間とのずれを測る
				const auto measureLine = [&](const int start, const int end, const auto& heightAt)
					{
						for (int a = start; a < end; a += stride)
						{
							const int b = min(a + stride, end);
							const float ha = heightAt(a);
							const float hb = heightAt(b);
							for (int v = a + 1; v < b; ++v)
							{
								const float t = static_cast<float>(v - a) / static_cast<float>(b - a);
								maxDeviation = max(maxDeviation, fabsf(heightAt(v) - (ha + (hb - ha) * t)));
							}
						}
					};
				measureLine(cxStart, cxEnd, [&](const int vx) { return heights[czStart * W + vx]; });
				measureLine(cxStart, cxEnd, [&](const int vx) { return heights[czEnd * W + vx]; });
				measureLine(czStart, czEnd, [&](const int vz) { return heights[vz * W + cxStart]; });
				measureLine(czStart, czEnd, [&](const int vz) { return heights[vz * W + cxEnd]; });
				return maxDeviation;
			}


			/** 影の高さマップで「影なし」を表す高さ */
			constexpr float NO_TERRAIN_SHADOW_HEIGHT = -1.0e6f;


			/**
			 * @brief 地形の影の高さマップをベイクする
			 * @details 各頂点から光の来る方向へハイトマップをたどり、
			 *          「この高さより下は地形の影になる」というワールドYを求める。
			 *          地面だけでなく、その上にいるキャラクターも高さを比べれば影か分かる。
			 *
			 * @param heights        頂点の高さ（ワールドYから yOffset を除いた値）。W×H 個
			 * @param W              横の頂点数
			 * @param H              縦の頂点数
			 * @param cellSizeX      頂点の間隔（ワールドX）
			 * @param cellSizeZ      頂点の間隔（3ds Max の奥行き。ワールドZとは符号が逆）
			 * @param yOffset        地形全体のYオフセット
			 * @param lightDirection ライトの向き（光が進む向き）
			 * @param outShadowHeights 影の高さ（ワールドY）の出力先。影ができない点は NO_TERRAIN_SHADOW_HEIGHT
			 * @param outStepCount   1頂点あたりの最大のたどる回数（ログ用）
			 */
			void BakeTerrainShadowHeights(
				const std::vector<float>& heights,
				int W,
				int H,
				float cellSizeX,
				float cellSizeZ,
				float yOffset,
				const Vector3& lightDirection,
				std::vector<float>& outShadowHeights,
				int& outStepCount
			)
			{
				outShadowHeights.assign(static_cast<size_t>(W) * H, NO_TERRAIN_SHADOW_HEIGHT);
				outStepCount = 0;

				// 光の来る方向（ワールド、Y-up）
				Vector3 toLight = lightDirection * -1.0f;
				toLight.Normalize();
				const float horizontal = sqrtf(toLight.x * toLight.x + toLight.z * toLight.z);
				// 真上からの光・地平線より下の光では地形の影はできない
				if (toLight.y <= 0.0f || horizontal < 1.0e-4f) return;

				// 水平に1進むごとに、光の線が上がる高さ
				const float slope = toLight.y / horizontal;

				float minHeight = FLT_MAX;
				float maxHeight = -FLT_MAX;
				for (const float h : heights)
				{
					minHeight = min(minHeight, h);
					maxHeight = max(maxHeight, h);
				}

				// 影が届く最大の水平距離までを、セル幅ずつたどる
				const float stepWorld = min(cellSizeX, cellSizeZ);
				const float maxDistance = (maxHeight - minHeight) / slope;
				const int numSteps = static_cast<int>(ceilf(maxDistance / stepWorld));
				outStepCount = numSteps;

				// 1歩あたりの頂点番号の進み。ワールドZ = -(originY + vz * cellSizeZ) なので Z は符号が逆
				const float stepVx = (toLight.x / horizontal) * stepWorld / cellSizeX;
				const float stepVz = -(toLight.z / horizontal) * stepWorld / cellSizeZ;
				const float riseVPerStep = stepWorld * slope;

				const auto sampleHeight = [&](const float fx, const float fz)
					{
						const int ix = static_cast<int>(fx);
						const int iz = static_cast<int>(fz);
						const int ix1 = min(ix + 1, W - 1);
						const int iz1 = min(iz + 1, H - 1);
						const float tx = fx - static_cast<float>(ix);
						const float tz = fz - static_cast<float>(iz);
						const float h00 = heights[iz * W + ix];
						const float h10 = heights[iz * W + ix1];
						const float h01 = heights[iz1 * W + ix];
						const float h11 = heights[iz1 * W + ix1];
						const float top = h00 + (h10 - h00) * tx;
						const float bottom = h01 + (h11 - h01) * tx;
						return top + (bottom - top) * tz;
					};

				for (int vz = 0; vz < H; ++vz)
				{
					for (int vx = 0; vx < W; ++vx)
					{
						float shadowHeight = NO_TERRAIN_SHADOW_HEIGHT;
						for (int k = 1; k <= numSteps; ++k)
						{
							const float fx = static_cast<float>(vx) + stepVx * static_cast<float>(k);
							const float fz = static_cast<float>(vz) + stepVz * static_cast<float>(k);
							if (fx < 0.0f || fz < 0.0f || fx > static_cast<float>(W - 1) || fz > static_cast<float>(H - 1)) break;

							const float drop = riseVPerStep * static_cast<float>(k);
							// これより先は一番高い山でも今の値を超えられない
							if (maxHeight - drop <= shadowHeight) break;

							shadowHeight = max(shadowHeight, sampleHeight(fx, fz) - drop);
						}
						if (shadowHeight > NO_TERRAIN_SHADOW_HEIGHT)
						{
							outShadowHeights[static_cast<size_t>(vz) * W + vx] = shadowHeight + yOffset;
						}
					}
				}
			}


			/**
			 * @brief ロード時間のログ（Logs/load_trace.txt）に1行追記する
			 */
			void AppendLoadTrace(const char* format, ...)
			{
				FILE* fp = nullptr;
				if (fopen_s(&fp, "Logs/load_trace.txt", "a") != 0 || fp == nullptr) return;
				va_list args;
				va_start(args, format);
				vfprintf(fp, format, args);
				va_end(args);
				fputc('\n', fp);
				fclose(fp);
			}


			/** QueryPerformanceCounter の値の差をミリ秒にする */
			double QpcToMs(const int64_t begin, const int64_t end)
			{
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				return 1000.0 * static_cast<double>(end - begin) / static_cast<double>(frequency.QuadPart);
			}


			/** 現在の QueryPerformanceCounter の値 */
			int64_t NowQpc()
			{
				LARGE_INTEGER now;
				QueryPerformanceCounter(&now);
				return now.QuadPart;
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
			// 描画側に渡したベイク影を無効にする（次のステージや地形の無い場面に残さない）
			if (m_isShadowBaked && nsBeastEngine::g_renderingEngine != nullptr)
			{
				nsBeastEngine::g_renderingEngine->ClearTerrainShadowMap();
			}
			for (auto& chunk : m_chunks)
			{
				for (auto& render : chunk.renders)
				{
					if (render)
						nsBeastEngine::OcclusionDitherManager::Get().Unregister(render.get());
				}
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
				m_initBeginQpc = NowQpc();
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
				BakeTerrainShadow();
				m_initStep++;
				break;

			case 3:
				if (InitRendererTextureStep()) m_initStep++;
				break;

			case 4:
				InitRendererPhysics();
				m_initStep++;
				break;

			case 5:
				if (InitRendererChunkStep()) m_initStep++;
				break;

			default:
				StartWrapper();
				m_isInited = true;
				AppendLoadTrace("Terrain Init (total): %.0f ms, descriptor heaps alive %d",
					QpcToMs(m_initBeginQpc, NowQpc()), nsK2EngineLow::g_numDescriptorHeapLive);
				break;
			}
			return m_isInited;
		}


		void TerrainObject::Update()
		{
			if (!m_isInited) return;
			m_modelRender.Update();
			for (auto& chunk : m_chunks)
			{
				for (auto& render : chunk.renders)
				{
					if (render) render->Update();
				}
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
			// チャンクごとに、メインカメラからの距離でLODを選んで1つだけ描く。
			// 影・サブビューも同じLODを使うので、影を落とす形と受ける形がずれない
			const Vector3 cameraPosition = CameraSystem::Get().GetMainCamera().GetPosition();
			const int numChunks = static_cast<int>(m_chunks.size());
			for (int i = 0; i < numChunks; i++)
			{
				// AABB上でカメラに最も近い点までの距離
				const nsK2EngineLow::AABB& aabb = m_chunkAABBs[i];
				const Vector3 nearest(
					max(aabb.GetMin().x, min(cameraPosition.x, aabb.GetMax().x)),
					max(aabb.GetMin().y, min(cameraPosition.y, aabb.GetMax().y)),
					max(aabb.GetMin().z, min(cameraPosition.z, aabb.GetMax().z)));
				const float distance = (nearest - cameraPosition).Length();

				int lod = 0;
				while (lod < ARRAYSIZE(CHUNK_LOD_DISTANCES) && distance >= CHUNK_LOD_DISTANCES[lod]) lod++;

				// 選んだLODが無ければ（ポリゴンが無いなど）細かい側へ戻る
				auto& renders = m_chunks[i].renders;
				while (lod > 0 && !renders[lod]) lod--;
				if (!renders[lod]) continue;

				// 地形の影をベイクしているなら、地形はカスケードシャドウに描かない（影を落とさない設定にしてある）
				if (m_isShadowBaked)
				{
					renders[lod]->Draw(rc);
					continue;
				}

				auto& farShadowRender = renders[FAR_CASCADE_SHADOW_LOD];
				if (lod < FAR_CASCADE_SHADOW_LOD && farShadowRender)
				{
					// 画面とカスケード0には選んだLOD、遠いカスケードの影には粗いLODを使う
					renders[lod]->SetShadowOnly(false);
					renders[lod]->SetShadowCascadeMask(1u);
					renders[lod]->Draw(rc);

					farShadowRender->SetShadowOnly(true);
					farShadowRender->SetShadowCascadeMask(nsBeastEngine::ModelRender::ALL_SHADOW_CASCADES_MASK & ~1u);
					farShadowRender->Draw(rc);
				}
				else
				{
					renders[lod]->SetShadowOnly(false);
					renders[lod]->SetShadowCascadeMask(nsBeastEngine::ModelRender::ALL_SHADOW_CASCADES_MASK);
					renders[lod]->Draw(rc);
				}
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

				m_chunks.clear();
				m_chunks.resize(numChunks);
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

						if (cxStart >= cxEnd || czStart >= czEnd) continue;

						// LODごとのチャンクメッシュ生成
						// スカートの長さ：隣のチャンクとLODが違うと、互いの縁が最大でこのずれだけ食い違う
						constexpr int coarsestStride = CHUNK_LOD_STRIDES[NUM_CHUNK_LODS - 1];
						const float edgeDeviation = CalcChunkEdgeDeviation(heights, W, cxStart, cxEnd, czStart, czEnd, coarsestStride);
						const float skirtDepth = edgeDeviation * 2.0f + max(cellSizeX, cellSizeZ);

						std::vector<TkmFile::SVertex> verts;
						std::vector<uint32_t> indices;
						for (int lod = 0; lod < NUM_CHUNK_LODS; ++lod)
						{
							BuildChunkLodMesh(heights, m_heightmap.pixels, W, H, cellSizeX, cellSizeZ, originX, originY, m_config,
								cxStart, cxEnd, czStart, czEnd, CHUNK_LOD_STRIDES[lod], skirtDepth, verts, indices);

							char key[64];
							snprintf(key, sizeof(key), "%s_lod%d_%d", TERRAIN_CHUNK_KEY_PREFIX, lod, chunkIdx);
							m_chunks[chunkIdx].tkmFiles[lod] = AssembleChunkTkm(std::move(verts), std::move(indices), key);
							verts.clear();
							indices.clear();
						}
					}
				}
			}

			// 影のベイク（次の初期化ステップ）で使う
			m_meshHeights = std::move(heights);
		}


		void TerrainObject::BakeTerrainShadow()
		{
			const int64_t beginQpc = NowQpc();

			const int W = m_heightmap.width;
			const int H = m_heightmap.height;
			if (W < 2 || H < 2 || m_meshHeights.size() != static_cast<size_t>(W) * H)
			{
				m_isShadowBaked = false;
				return;
			}

			// GenerateMesh() と同じ配置（3ds Max Z-up → DX Y-up: world_x=src_x, world_y=src_z, world_z=-src_y）
			const float cellSizeX = m_config.totalWidth / float(W - 1);
			const float cellSizeZ = m_config.totalDepth / float(H - 1);
			const float originX = -m_config.totalWidth * 0.5f;
			const float originY = -m_config.totalDepth * 0.5f;

			const Vector3 lightDirection =
				nsBeastEngine::g_renderingEngine->GetSceneLight().GetLight()->m_directionLight.m_direction;

			std::vector<float> shadowHeights;
			int stepCount = 0;
			BakeTerrainShadowHeights(m_meshHeights, W, H, cellSizeX, cellSizeZ, m_config.yOffset, lightDirection, shadowHeights, stepCount);
			const int64_t bakedQpc = NowQpc();

			// ワールドXZ → マップのUV（テクセルの中心に頂点が来るように半テクセルずらす）
			//   u = (vx + 0.5) / W,  vx = (worldX - originX) / cellSizeX
			//   v = (vz + 0.5) / H,  vz = (-worldZ - originY) / cellSizeZ
			const Vector4 uvScaleOffset(
				1.0f / (cellSizeX * W),
				(0.5f - originX / cellSizeX) / W,
				-1.0f / (cellSizeZ * H),
				(0.5f - originY / cellSizeZ) / H);
			// （影の高さ, 地面の高さ）を並べる。地面の高さは、LODで間引いた面でも本来の地面で影を判定するのに使う
			std::vector<float> texels(static_cast<size_t>(W) * H * 2);
			for (size_t i = 0; i < shadowHeights.size(); ++i)
			{
				texels[i * 2 + 0] = shadowHeights[i];
				texels[i * 2 + 1] = m_meshHeights[i] + m_config.yOffset;
			}
			nsBeastEngine::g_renderingEngine->SetTerrainShadowMap(
				texels, W, H, uvScaleOffset, TERRAIN_SHADOW_SOFTNESS, TERRAIN_SHADOW_BIAS);
			m_isShadowBaked = true;

			// 使い終わった高さは捨てる
			m_meshHeights.clear();
			m_meshHeights.shrink_to_fit();

			const int64_t endQpc = NowQpc();
			AppendLoadTrace("Terrain ShadowBake: %.0f ms (bake %.0f ms + upload %.0f ms, %dx%d, steps %d)",
				QpcToMs(beginQpc, endQpc), QpcToMs(beginQpc, bakedQpc), QpcToMs(bakedQpc, endQpc), W, H, stepCount);
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

			// チャンク×LOD を1つずつ初期化する
			const int numChunks = static_cast<int>(m_chunks.size());
			const int numSteps = numChunks * NUM_CHUNK_LODS;

			LARGE_INTEGER freq, begin, now;
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&begin);

			while (m_chunkInitIndex < numSteps)
			{
				const int step = m_chunkInitIndex++;
				const int i = step / NUM_CHUNK_LODS;
				const int lod = step % NUM_CHUNK_LODS;
				if (m_chunks[i].tkmFiles[lod] == nullptr) continue;  // minHeight で全クワッドが除外されたチャンク

				char key[64];
				snprintf(key, sizeof(key), "%s_lod%d_%d", TERRAIN_CHUNK_KEY_PREFIX, lod, i);

				nsK2EngineLow::ModelInitData initData;
				SetupModelInitData(initData, key);
				auto& render = m_chunks[i].renders[lod];
				render = std::make_unique<nsBeastEngine::ModelRender>();
				render->SetGBufferFxFilePath("Assets/shader/Terrain.fx");
				// 地形の影をベイクしているなら影は描かないので、影用モデルを作らない。
				// チャンク×LODで数が多く、作るとディスクリプタヒープが上限に近づく（フィーバー中に作成失敗した）
				if (m_isShadowBaked)
				{
					render->SetCreateShadowModels(false);
				}
				render->InitFromLoaded(initData);
				render->SetPBRParam(m_config.pbrParam);
				render->SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
				render->Update();
				nsBeastEngine::OcclusionDitherManager::Get().Register(render.get());

				// 地形の影をベイクしているなら、地形はカスケードシャドウに描かない
				// ベイクしていなければ、平らなチャンクだけ影を落とさない（影は自分自身にしか落ちないため）
				// ※モデルのAABBはスカートで下に伸びているので、ハイトマップから求めたチャンクの高さの範囲で判定する
				const float chunkHeightRange = m_chunkAABBs[i].GetMax().y - m_chunkAABBs[i].GetMin().y;
				if (m_isShadowBaked ||
					(chunkHeightRange >= 0.0f && chunkHeightRange < FLAT_CHUNK_SHADOW_HEIGHT_RANGE))
				{
					render->SetCastShadow(false);
				}

				QueryPerformanceCounter(&now);
				const double elapsedMs = 1000.0 * (now.QuadPart - begin.QuadPart) / freq.QuadPart;
				if (elapsedMs > CHUNK_INIT_BUDGET_MS) break;
			}
			return m_chunkInitIndex >= numSteps;
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
