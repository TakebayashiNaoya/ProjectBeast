/**
 * @file TerrainObject.h
 * @brief ハイトマップから生成する地形オブジェクト
 */
#pragma once
#include "Source/Actor/Actor.h"
#include "Physics/PhysicalBody.h"


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
				float totalWidth  = 5000.0f;  ///< 地形 X 方向の総幅（ワールド単位）
				float totalDepth  = 5000.0f;  ///< 地形 Z 方向の総奥行き（ワールド単位）
				float heightScale = 500.0f;   ///< ハイトマップ最大高さ（ワールド単位）
				int   subsample   = 8;        ///< サブサンプル倍率（1=フル解像度、大=低ポリゴン数）
				float uvTile      = 0.05f;    ///< テクスチャタイリング係数（1/uvTile セルごとに繰り返し）
				float albedoScale = 1.0f;     ///< アルベド明度スケール（1.0=そのまま、小さくすると暗くなる）
				float yOffset     = 0.0f;     ///< 地形全体の Y 座標オフセット（負=下げる、正=上げる）
				float minHeight   = 0.0f;     ///< この高さ未満の頂点を含むクワッドはポリゴンを生成しない（ワールド単位）
			};

			/**
			 * @brief 地形の定数バッファ（b1 レジスタ）
			 * @details Terrain.fx の TerrainCb と一致させること。
			 */
			struct TerrainCb
			{
				float halfWidth   = 0.0f;   // 地形の X 方向半幅（ワールド単位）
				float halfDepth   = 0.0f;   // 地形の Z 方向半奥行き（ワールド単位）
				float albedoScale = 1.0f;   // アルベド明度スケール
				float pad         = {};
			};

		public:
			TerrainObject()  = default;
			~TerrainObject() = default;

			/**
			 * @brief 地形を初期化する
			 * @param config 地形パラメータ（省略時はデフォルト値）
			 */
			void Init(const TerrainConfig& config = {});

		protected:
			void Start()  override {}
			void Update() override;
			void Render(RenderContext& rc) override;

		private:
			/** ハイトマップの CPU 側データ */
			struct HeightmapData
			{
				std::vector<uint16_t> pixels;
				int width  = 0;
				int height = 0;
			};

			/** DDS R16_UNORM をファイルから CPU に読み込む */
			void LoadHeightmap();

			/** HeightmapData から TkmFile を構築し、バンクに登録する */
			void GenerateMesh();

			/** テクスチャをロードし ModelRender を初期化する */
			void InitRenderer();

		private:
			HeightmapData           m_heightmap;
			// バンクに new で渡して所有権を委譲するため生ポインタで保持（delete 禁止）
			nsK2EngineLow::TkmFile* m_tkmFile = nullptr;

			nsK2EngineLow::Texture m_splatmap;
			nsK2EngineLow::Texture m_terrainTextures[4];  // [0]=snow [1]=glass [2]=rock [3]=snow fallback
			nsK2EngineLow::Texture m_snowNormal;           // 雪ノーマルマップ    (t15)
			nsK2EngineLow::Texture m_snowRoughness;        // 雪ラフネスマップ    (t16)

			nsBeastEngine::nsCollision::PhysicalBody m_physicalBody;

			TerrainConfig m_config;
			TerrainCb     m_terrainCb;

			bool m_isInited = false;
		};
	}
}
