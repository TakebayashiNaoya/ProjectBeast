/**
 * @file FormationRangeVisualizer.h
 * @brief 陣形の入隊・脱隊半径を地形追従ラインリングで可視化する
 * @author 竹林
 */
#pragma once
#include <vector>
#include "Graphics/ICustomRenderer.h"
#include "TerrainCircle.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形範囲ビジュアライザー（地形追従ライン）
		 * @details
		 *   入隊半径（緑）と脱隊半径（赤）を TerrainCircle で描画し、
		 *   フォーメーションの各スロット位置を白い小円で可視化する。
		 *   地形追従・半透明塗りつぶしは TerrainCircle が担当し、
		 *   本クラスは PSO・ルートシグネチャ・定数バッファ等の共有 GPU リソースを保持する。
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
			 * @param center         中心座標（親ペンギンの座標）
			 * @param joinRadius     入隊半径
			 * @param leaveRadius    脱隊半径
			 * @param slotPositions  スロット座標リスト（次レベル分の全座標）
			 */
			void Update(const Vector3& center, float joinRadius, float leaveRadius, const std::vector<Vector3>& slotPositions);

			/**
			 * @brief 描画
			 * @param rc   レンダリングコンテキスト
			 * @param view 描画対象ビュー（カメラ行列の取得に使用）
			 */
			void Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view) override;


		private:
			/** ルートシグネチャの初期化 */
			void InitRootSignature();
			/** シェーダーの初期化 */
			void InitShader();
			/** 縁取り用パイプラインステートの初期化（LINE_LIST） */
			void InitLinePipelineState();
			/** 塗りつぶし用パイプラインステートの初期化（TRIANGLE_LIST + αブレンド） */
			void InitFillPipelineState();
			/** 定数バッファの初期化 */
			void InitConstantBuffer();
			/** ディスクリプタヒープの初期化 */
			void InitDescriptorHeap();


		private:
			static const Vector4 JOIN_EDGE_COLOR;   /** 緑（入隊半径・縁） */
			static const Vector4 JOIN_FILL_COLOR;   /** 緑（入隊半径・塗りつぶし） */
			static const Vector4 LEAVE_EDGE_COLOR;  /** 赤（脱隊半径・縁） */
			static const Vector4 LEAVE_FILL_COLOR;  /** 赤（脱隊半径・塗りつぶし） */
			static const Vector4 SLOT_COLOR;        /** 白（スロットマーカー） */
						
			static constexpr int   RANGE_SEGS	   = 32;	/** 入隊・脱隊半径リングの分割数 */
			static constexpr int   SLOT_SEGS       = 12;	/** スロットマーカーの分割数 */
			static constexpr float SLOT_RADIUS     = 5.0f;  /** スロットマーカーの半径 */
			static constexpr int   MAX_SLOT_COUNT  = 100;	/** スロットマーカーの事前確保数（一周分 = リング k のスロット数 k*9、リング11まで対応） */

			TerrainCircle m_joinCircle;					  /** 入隊半径円（緑・塗りつぶしあり） */
			TerrainCircle m_leaveCircle;				  /** 脱隊半径円（赤・塗りつぶしあり） */
			TerrainCircle m_slotCircles[MAX_SLOT_COUNT];  /** スロットマーカー円（Init()で全て事前確保） */
			int m_activeSlotCount = 0;					  /** 現フレームで描画するスロットマーカーの個数 */

			RootSignature  m_rootSignature;      /** ルートシグネチャ */
			Shader         m_vs;                 /** 頂点シェーダ */
			Shader         m_ps;                 /** ピクセルシェーダ */
			PipelineState  m_linePipelineState;  /** 縁取り用パイプラインステート */
			PipelineState  m_fillPipelineState;  /** 塗りつぶし用パイプラインステート */
			ConstantBuffer m_constantBuffer;     /** 定数バッファ（VP 行列） */
			DescriptorHeap m_descriptorHeap;     /** ディスクリプタヒープ */

			bool m_isInitialized = false;        /** 初期化済みかどうか */
			bool m_isVisible     = true;         /** 可視化有効かどうか */
		};
	}
}
