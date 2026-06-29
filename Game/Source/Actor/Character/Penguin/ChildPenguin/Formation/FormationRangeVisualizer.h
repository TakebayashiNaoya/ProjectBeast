/**
 * @file FormationRangeVisualizer.h
 * @brief 陣形の入隊・脱隊半径を地形追従ラインリングで可視化する
 * @author 竹林
 */
#pragma once
#include <array>
#include <cstdint>
#include "Graphics/ICustomRenderer.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形範囲ビジュアライザー（地形追従ライン）
		 * @details
		 *   入隊半径（緑）と脱隊半径（赤）を円周上の N_SEGMENTS 点を結ぶラインリングとして描画する。
		 *   各頂点の Y 座標は PhysicsWorld::Raycast() で地形をサンプリングし、
		 *   地形がなければ Ocean::SampleWaveHeight() で波面をサンプリングする。
		 *   Update() で毎フレーム動的頂点バッファを更新するため、海面の波打ちにも追従する。
		 */
		class FormationRangeVisualizer : public nsBeastEngine::ICustomRenderer
		{
		public:
			/**
			 * @brief 可視化の有効/無効を設定
			 * @param visible true で可視化有効、false で無効
			 */
			void SetVisible(bool visible) { m_isVisible = visible; }
			/**
			 * @brief 可視化の有効/無効を取得
			 * @return true で可視化有効、false で無効
			 */
			bool IsVisible() const { return m_isVisible; }


		public:
			/** 
			 * @brief 初期化
			 */
			void Init();

			/**
			 * @brief 更新
			 * @param center      中心座標（親ペンギンの座標）
			 * @param joinRadius  入隊半径
			 * @param leaveRadius 脱隊半径
			 */
			void Update(const Vector3& center, float joinRadius, float leaveRadius);

			/**
			 * @brief 描画
			 * @param rc   レンダリングコンテキスト
			 * @param view 描画対象ビュー（カメラ行列の取得に使用）
			 */
			void Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view) override;


		private:
			/** 円周上 1 点の地表 Y をレイキャスト → 海面の順にサンプリングして返す */
			float SampleSurfaceY(float x, float z) const;

			/** 指定半径・色でリング縁取り頂点を m_vertices に追記する */
			void BuildRingVertices(const Vector3& center, float radius, const Vector4& color);
			/** 指定半径・色でリング塗りつぶし頂点を m_fillVertices に追記する */
			void BuildRingFillVertices(const Vector3& center, float radius, const Vector4& color);

			/** ルートシグネチャの初期化 */
			void InitRootSignature();
			/** シェーダーの初期化 */
			void InitShader();
			/** パイプラインステートの初期化 */
			void InitPipelineState();
			/** 頂点バッファの初期化 */
			void InitVertexBuffer();
			/** インデックスバッファの初期化 */
			void InitIndexBuffer();
			/** 塗りつぶし頂点バッファの初期化 */
			void InitFillVertexBuffer();
			/** 塗りつぶしインデックスバッファの初期化 */
			void InitFillIndexBuffer();
			/** 塗りつぶし用パイプラインステートの初期化 */
			void InitFillPipelineState();
			/** 定数バッファの初期化 */
			void InitConstantBuffer();
			/** ディスクリプタヒープの初期化 */
			void InitDescriptorHeap();


		private:
			/** 頂点構造体 */
			struct Vertex
			{
				Vertex() : pos(0.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f, 1.0f) {}
				Vertex(const Vector3& p, const Vector4& c) : pos(p), color(c) {}
				Vector3 pos;
				Vector4 color;
			};

			/** 1 リングあたりの分割数。増やすほど滑らかになるが Raycast 呼び出し数も増える */
			static constexpr int   N_SEGMENTS          = 32;
			/** LINE_LIST: 1 セグメント = 始点 + 終点 = 2 頂点 */
			static constexpr int   VERTS_PER_RING      = N_SEGMENTS * 2;
			/** 入隊リング + 脱隊リング */
			static constexpr int   TOTAL_VERTS         = VERTS_PER_RING * 2;
			/** TRIANGLE_LIST: 1 セグメント = 中心 + 始点 + 終点 = 3 頂点 */
			static constexpr int   FILL_VERTS_PER_RING = N_SEGMENTS * 3;
			/** 入隊リング + 脱隊リング（塗りつぶし） */
			static constexpr int   TOTAL_FILL_VERTS    = FILL_VERTS_PER_RING * 2;
			/** Z-fighting 回避のため地表から少し浮かせる量（cm 単位想定） */
			static constexpr float HEIGHT_OFFSET  = 3.0f;
			static constexpr float RAYCAST_TOP    = 1000.0f;
			static constexpr float RAYCAST_BOTTOM = -10.0f;

			static const Vector4 JOIN_COLOR;        /** 緑（入隊半径・縁） */
			static const Vector4 LEAVE_COLOR;       /** 赤（脱隊半径・縁） */
			static const Vector4 JOIN_FILL_COLOR;   /** 緑（入隊半径・塗りつぶし） */
			static const Vector4 LEAVE_FILL_COLOR;  /** 赤（脱隊半径・塗りつぶし） */

			std::array<Vertex, TOTAL_VERTS>      m_vertices     = {};	/** 縁取り用頂点配列 */
			int                                  m_vertexCount  = 0;	/** 現在の縁取り頂点数 */
			std::array<Vertex, TOTAL_FILL_VERTS> m_fillVertices    = {};	/** 塗りつぶし用頂点配列 */
			int                                  m_fillVertexCount = 0;	/** 現在の塗りつぶし頂点数 */

			VertexBuffer   m_vertexBuffer;		/** 縁取り頂点バッファ */
			IndexBuffer    m_indexBuffer;		/** 縁取りインデックスバッファ */
			VertexBuffer   m_fillVertexBuffer;	/** 塗りつぶし頂点バッファ */
			IndexBuffer    m_fillIndexBuffer;	/** 塗りつぶしインデックスバッファ */
			RootSignature  m_rootSignature;		/** ルートシグネチャ */
			Shader         m_vs;				/** 頂点シェーダ */
			Shader         m_ps;				/** ピクセルシェーダ */
			PipelineState  m_pipelineState;		/** 縁取り用パイプラインステート（LINE_LIST） */
			PipelineState  m_fillPipelineState;	/** 塗りつぶし用パイプラインステート（TRIANGLE_LIST + αブレンド） */
			ConstantBuffer m_constantBuffer;	/** 定数バッファ */
			DescriptorHeap m_descriptorHeap;	/** ディスクリプタヒープ */

			bool m_isInitialized = false;		/** 初期化済みかどうか */
			bool m_isVisible	 = true;		/** 可視化有効かどうか */
		};
	}
}
