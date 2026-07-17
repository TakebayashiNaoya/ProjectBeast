/**
 * @file TerrainObject.h
 * @brief ハイトマップから生成する地形オブジェクト
 * @author 竹林
 */
#pragma once
#include "Physics/PhysicalBody.h"
#include "Source/Actor/Actor.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ハイトマップ DDS + スプラットマップ DDS から地形メッシュを生成するクラス
		 *
		 * アセットパスは TerrainObject.cpp 内の定数で管理する。
		 * b1 レジスタに TerrainCb（地形の半幅・半奥行き）を渡し、
		 * Terrain.fx 側でスプラットマップ UV を計算する。
		 * 拡張 SRV（t10〜t14）にスプラットマップと 4 種のテクスチャをバインドする。
		 */
		class TerrainObject : public Actor
		{
		public:
			/** @brief 地形生成パラメータ。Init() に渡して挙動を制御する。 */
			struct TerrainConfig
			{
				float totalWidth = 5000.0f;  ///< 地形 X 方向の総幅（ワールド単位）
				float totalDepth = 5000.0f;  ///< 地形 Z 方向の総奥行き（ワールド単位）
				float heightScale = 500.0f;   ///< ハイトマップ最大高さ（ワールド単位）
				int   subsample = 8;        ///< サブサンプル倍率（1=フル解像度、大=低ポリゴン数）
				float uvTile = 0.05f;    ///< テクスチャタイリング係数（1/uvTile セルごとに繰り返し）
				float albedoScale = 1.0f;     ///< アルベド明度スケール（1.0=そのまま、小さくすると暗くなる）
				float yOffset = 0.0f;     ///< 地形全体の Y 座標オフセット（負=下げる、正=上げる）
				float minHeight = 0.0f;     ///< この高さ未満の頂点を含むクワッドはポリゴンを生成しない（ワールド単位）
				int   chunkDivision = 8;      ///< フラスタムカリング用チャンク分割数（縦横共通）
				nsBeastEngine::PBRParam pbrParam;  ///< PBR補正パラメータ（ModelRender::SetPBRParam に渡す）
				std::wstring heightmapPath = L"Assets/modelData/stage/Terrain/TutorialStageHeightMap.dds";  ///< ハイトマップ DDS パス
				std::wstring splatmapPath = L"Assets/modelData/stage/Terrain/TutorialStageSplatMap.dds";   ///< スプラットマップ DDS パス
			};

			/**
			 * @brief 地形の定数バッファ（b1 レジスタ）
			 * @details Terrain.fx の TerrainCb と一致させること。
			 */
			struct TerrainCb
			{
				float halfWidth = 0.0f;   // 地形の X 方向半幅（ワールド単位）
				float halfDepth = 0.0f;   // 地形の Z 方向半奥行き（ワールド単位）
				float albedoScale = 1.0f;   // アルベド明度スケール
				float pad = {};     // パディング（16バイトアライメント）
			};

		public:
			TerrainObject() = default;
			~TerrainObject();

			/**
			 * @brief 地形を初期化する
			 * @param config 地形パラメータ（省略時はデフォルト値）
			 */
			void Init(const TerrainConfig& config = {});

			/** @brief 地表の種類（スプラットマップのR/G/Bに対応。R=雪, G=草, B=岩） */
			enum class SurfaceType
			{
				Snow,
				Grass,
				Rock
			};

			/**
			 * @brief 指定したワールド座標の地表種別を取得する（スプラットマップをCPU側から参照）
			 * @param worldPos ワールド座標
			 * @return 地表種別（未初期化・範囲外の場合は Snow を返す）
			 */
			SurfaceType GetSurfaceTypeAt(const Vector3& worldPos) const;

			/**
			 * @brief 指定したワールドXZ座標の地形の高さ（ワールドY）を取得する（バイリニア補間）
			 * @details 投影デカールの格子メッシュを地形に沿わせるために使用する。
			 * @param worldPos ワールド座標（Yは無視される）
			 * @return ワールド空間の高さ（Y座標）。未初期化の場合は0を返す
			 */
			float GetHeightAt(const Vector3& worldPos) const;

			/** @brief 足跡デカールが地形の高さをGPU上でサンプリングするためのハイトマップテクスチャを取得する */
			nsK2EngineLow::Texture& GetHeightmapTextureGpu() { return m_heightmapTextureGpu; }

			// ★追加: デカール側でワールド座標→UV変換をするために必要な値を公開する
			float GetHalfWidth()   const { return m_terrainCb.halfWidth; }
			float GetHalfDepth()   const { return m_terrainCb.halfDepth; }
			float GetHeightScale() const { return m_config.heightScale; }
			float GetYOffset()     const { return m_config.yOffset; }

		protected:
			void Start()  override {}
			void Update() override;
			void Render(RenderContext& rc) override;

		private:
			/** ハイトマップの CPU 側データ */
			struct HeightmapData
			{
				std::vector<uint16_t> pixels;
				int width = 0;
				int height = 0;
			};

			/** @brief スプラットマップのCPU側コピー（GetSurfaceTypeAt専用） */
			struct SplatmapCpuData
			{
				std::vector<uint8_t> pixels; // RGBA8
				int width = 0;
				int height = 0;
			};

			/** @brief DDSファイルからスプラットマップをCPUに読み込む（GetSurfaceTypeAt用） */
			void LoadSplatmapCpu();

			/** @brief ハイトマップ画素座標(浮動小数)からバイリニア補間で高さを取得する内部ヘルパー */
			float SampleHeightBilinear(float fx, float fz) const;

			SplatmapCpuData m_splatmapCpu;

			/** DDS R16_UNORM をファイルから CPU に読み込む */
			void LoadHeightmap();

			/** HeightmapData からチャンク別 TkmFile を構築し、バンクに登録する */
			void GenerateMesh();

			/** テクスチャをロードし ModelRender を初期化する */
			void InitRenderer();

		private:
			HeightmapData           m_heightmap;

			// 物理コリジョン用フルメッシュ（描画には使わない）
			nsK2EngineLow::TkmFile* m_tkmFile = nullptr;

			// チャンク描画用（フラスタムカリング対象）
			std::vector<nsK2EngineLow::TkmFile*>                       m_chunkTkmFiles;
			std::vector<std::unique_ptr<nsBeastEngine::ModelRender>>   m_chunkRenders;
			std::vector<nsK2EngineLow::AABB>                           m_chunkAABBs;

			nsK2EngineLow::Texture m_splatmap;
			nsK2EngineLow::Texture m_terrainTextures[3];  // [0]=snow [1]=grass [2]=rock
			nsK2EngineLow::Texture m_snowNormal;           // 雪ノーマルマップ (t15)
			nsK2EngineLow::Texture m_snowRoughness;        // 雪ラフネスマップ (t16)
			nsK2EngineLow::Texture m_grassNormal;          // 草ノーマルマップ (t17)
			nsK2EngineLow::Texture m_grassRoughness;       // 草ラフネスマップ (t18)
			nsK2EngineLow::Texture m_rockNormal;           // 岩ノーマルマップ (t19)
			nsK2EngineLow::Texture m_rockRoughness;        // 岩ラフネスマップ (t20)

			nsBeastEngine::nsCollision::PhysicalBody m_physicalBody;  // 当たり判定

			TerrainConfig m_config;		// 地形生成パラメータ
			TerrainCb     m_terrainCb;	// 地形定数バッファ（b1 レジスタ）

			bool m_isInited = false;	// 初期化済みフラグ

			// シェーダーでサンプリングするためのGPUテクスチャは別途フル解像度で持つ
			nsK2EngineLow::Texture m_heightmapTextureGpu;
		};
	}
}
