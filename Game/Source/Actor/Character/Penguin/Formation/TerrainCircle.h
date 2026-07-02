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
			 * @param center 円の中心座標（XZ 座標から地表 Y をサンプリングする）
			 * @param radius 円の半径
			 */
			void Update(const Vector3& center, float radius);

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
		};
	}
}
