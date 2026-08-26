/**
 * @file TerrainCircle.h
 * @brief 地形に追従し、内側を塗りつぶせる円の描画クラス
 * @author 竹林
 */
#pragma once
#include <vector>


namespace app
{
	namespace actor
	{
		/**
		 * @brief 地形追従円
		 * @details
		 *   Update() で中心座標と半径を受け取り、PhysicsWorld::Raycast と
		 *   Ocean::SampleWaveHeight で各頂点の地表 Y をサンプリングして
		 *   地形に貼り付いた円を生成する。
		 *   PSO・ルートシグネチャ・定数バッファは所有しない（FormationRangeVisualizer から渡される）。
		 */
		class TerrainCircle
		{
		public:
			/**
			 * @brief 頂点構造体（FormationRangeVisualizer の PSO 入力レイアウトと一致）
			 */
			struct Vertex
			{
				Vector3 pos;
				Vector4 color;
			};


		public:
			/**
			 * @brief 初期化
			 * @param segments  円周の分割数
			 * @param edgeColor 縁取りの色（RGBA）
			 * @param fillColor 塗りつぶしの色（RGBA）。hasFill=false の場合は使用しない
			 * @param hasFill   true のとき内側を TRIANGLE_LIST で塗りつぶす
			 */
			void Init(int segments, const Vector4& edgeColor, const Vector4& fillColor, bool hasFill);

			/**
			 * @brief 毎フレーム更新
			 * @param center     円の中心座標（XZ 座標から地表 Y をサンプリングする）
			 * @param radius     円の半径
			 * @param alphaScale 縁取り・塗りつぶしの不透明度に掛ける係数（波紋のフェード用）
			 */
			void Update(const Vector3& center, float radius, float alphaScale = 1.0f);

			/**
			 * @brief グラデーション帯（波紋リング）の初期化
			 * @details 中央が濃く、内外の縁へ向かって透明になる帯を TRIANGLE_LIST で描く。
			 *          Init() とは独立して使える（両方初期化してもよい）
			 * @param segments 円周の分割数
			 * @param color    帯の色（RGBA。アルファは帯中央の濃さ）
			 */
			void InitBand(int segments, const Vector4& color);

			/**
			 * @brief グラデーション帯の色を変更する
			 * @details 陣形色のウルトリングなど、実行時に色を切り替えたい帯で使う
			 * @param color 帯の色（RGBA。アルファは帯中央の濃さ）
			 */
			void SetBandColor(const Vector4& color) { m_bandColor = color; }

			/**
			 * @brief グラデーション帯の毎フレーム更新
			 * @param center     帯の中心座標
			 * @param radius     帯の中心線の半径
			 * @param halfWidth  帯の半幅（radius±halfWidth の範囲に描く）
			 * @param alphaScale 帯全体の不透明度係数（波紋のフェード用）
			 */
			void UpdateBand(const Vector3& center, float radius, float halfWidth, float alphaScale);

			/**
			 * @brief グラデーション帯を描画する（TRIANGLE_LIST）
			 * @note 呼び出し前に塗りつぶし用 PSO（アルファブレンド有効）がセットされていること
			 */
			void RenderBand(RenderContext& rc);

			/**
			 * @brief 塗りつぶしを描画する（TRIANGLE_LIST）
			 * @note 呼び出し前に塗りつぶし用 PSO がセットされていること
			 */
			void RenderFill(RenderContext& rc);

			/**
			 * @brief 縁取りを描画する（LINE_LIST）
			 * @note 呼び出し前に縁取り用 PSO がセットされていること
			 */
			void RenderEdge(RenderContext& rc);

			/**
			 * @brief 塗りつぶしを持つかどうか
			 * @return true のとき内側を TRIANGLE_LIST で塗りつぶす
			 */
			bool HasFill()     const { return m_hasFill; }

			/**
			 * @brief Update 済みで描画可能かどうか
			 * @return true のとき描画可能
			 */
			bool HasVertices() const { return m_edgeVertexCount > 0; }


		private:
			/**
			 * @brief 指定 XZ 座標の地表 Y をサンプリングする
			 * @details PhysicsWorld::Raycast → Ocean::SampleWaveHeight の順に試みる
			 */
			static float SampleSurfaceY(float x, float z);


		private:
			static constexpr float RAYCAST_TOP    = 1000.0f;  /** レイキャストの上方向 */
			static constexpr float RAYCAST_BOTTOM = -10.0f;	  /** レイキャストの下方向 */
			static constexpr float HEIGHT_OFFSET  = 3.0f;     /** 地表 Y からのオフセット（円が地面にめり込まないようにする） */

			int     m_segments  = 32;						  /** 円周の分割数 */
			bool    m_hasFill   = true;						  /** 内側を塗りつぶすかどうか */
			Vector4 m_edgeColor = { 1.0f, 1.0f, 1.0f, 1.0f }; /** 縁取りの色 */
			Vector4 m_fillColor = { 1.0f, 1.0f, 1.0f, 0.5f }; /** 塗りつぶしの色 */

			std::vector<Vertex> m_edgeVerts;  /** 縁取り用頂点バッファ */
			std::vector<Vertex> m_fillVerts;  /** 塗りつぶし用頂点バッファ */
			int m_edgeVertexCount = 0;		  /** 縁取り用頂点バッファの有効頂点数 */
			int m_fillVertexCount = 0;		  /** 塗りつぶし用頂点バッファの有効頂点数 */

			VertexBuffer m_edgeVertexBuffer;  /** 縁取り用頂点バッファ */
			IndexBuffer  m_edgeIndexBuffer;   /** 縁取り用インデックスバッファ */
			VertexBuffer m_fillVertexBuffer;  /** 塗りつぶし用頂点バッファ */
			IndexBuffer  m_fillIndexBuffer;   /** 塗りつぶし用インデックスバッファ */

			int     m_bandSegments = 0;                       /** 帯の円周分割数（0なら帯は未初期化） */
			Vector4 m_bandColor = { 1.0f, 1.0f, 1.0f, 1.0f }; /** 帯の色 */
			std::vector<Vertex> m_bandVerts;                  /** 帯用頂点バッファ */
			int m_bandVertexCount = 0;                        /** 帯用頂点バッファの有効頂点数 */
			VertexBuffer m_bandVertexBuffer;                  /** 帯用頂点バッファ */
			IndexBuffer  m_bandIndexBuffer;                   /** 帯用インデックスバッファ */
		};
	}
}
