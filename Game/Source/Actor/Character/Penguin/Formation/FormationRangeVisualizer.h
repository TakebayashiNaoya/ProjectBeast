/**
 * @file FormationRangeVisualizer.h
 * @brief 再集合の呼びかけ範囲を地形追従ラインリングで可視化する
 */
#pragma once
#include <vector>
#include "Graphics/ICustomRenderer.h"
#include "TerrainCircle.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 再集合の呼びかけ範囲ビジュアライザー（地形追従ライン）
		 * @details
		 *   Yボタンの呼びかけが届く範囲（緑）を TerrainCircle で描画する。
		 *   もとは陣形の入隊半径の常時表示だったが、呼びかけ中の範囲表示に転用した。
		 *   地形追従・半透明塗りつぶしは TerrainCircle が担当し、
		 *   本クラスは PSO・ルートシグネチャ・定数バッファ等の共有 GPU リソースを保持する。
		 */
		class FormationRangeVisualizer : public nsBeastEngine::ICustomRenderer
		{
		public:
			/**
			 * @brief 可視化の有効/無効を設定
			 * @details 非表示→表示の瞬間に波紋の位相を0に戻し、
			 *          呼びかけのたびに外周から波紋が出はじめるようにする
			 * @param visible true で可視化有効、false で無効
			 */
			void SetVisible(bool visible)
			{
				if (visible && !m_isVisible) m_rippleTimer = 0.0f;
				m_isVisible = visible;
			}
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
			 * @param joinRadius     表示半径（再集合の呼びかけが届く半径）
			 * @param slotPositions  スロット座標リスト（次レベル分の全座標）
			 */
			void Update(const Vector3& center, float joinRadius, const std::vector<Vector3>& slotPositions);

			/**
			 * @brief 演出の大きさ合わせ用の円を描く（調整用）
			 * @param center 中心座標
			 * @param radius 半径（0以下で消す）
			 * @param type   陣形（円の色を選ぶ）
			 */
			void SetFitCircle(const Vector3& center, float radius, EnFormationType type);

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
			static const Vector4 BOUNDARY_COLOR;    /** 薄いオレンジ（範囲の外周の帯） */
			static const Vector4 RIPPLE_COLOR;      /** オレンジ（収束する波紋の帯） */

			static constexpr int   RANGE_SEGS = 32;  /** 範囲リングの分割数 */

			static constexpr int   RIPPLE_COUNT        = 3;      /** 同時に出す波紋の本数 */
			static constexpr float RIPPLE_PERIOD       = 0.9f;   /** 波紋1本が外周から中心へ届くまでの時間（秒） */
			static constexpr float RIPPLE_INNER_RATIO  = 0.08f;  /** 波紋が消える半径（外周半径に対する比） */
			static constexpr float RIPPLE_FADE_IN_END  = 0.12f;  /** 進行度がここまでは出現フェードイン */
			static constexpr float RIPPLE_HALF_WIDTH   = 55.0f;  /** 波紋の帯の半幅（グラデーションの広がり） */
			static constexpr float BOUNDARY_HALF_WIDTH = 22.0f;  /** 外周の帯の半幅 */

			// TODO: スロットマーカー実装時に以下を有効化する。
			//       合わせて Init() / Update() / Render() 内のコメントアウト部分も有効化すること。
			//static const Vector4 SLOT_COLOR;                  /** 白（スロットマーカー） */
			//static constexpr int   SLOT_SEGS      = 12;      /** スロットマーカーの分割数 */
			//static constexpr float SLOT_RADIUS    = 5.0f;    /** スロットマーカーの半径 */
			//static constexpr int   MAX_SLOT_COUNT = 100;     /** スロットマーカーの事前確保数（リング k のスロット数 k*9、リング11まで対応） */
			//TerrainCircle m_slotCircles[MAX_SLOT_COUNT];      /** スロットマーカー円 */
			//int           m_activeSlotCount = 0;              /** 現フレームで描画するスロットマーカーの個数 */

			TerrainCircle m_joinCircle;                   /** 範囲の外周の帯（薄いオレンジ） */
			TerrainCircle m_rippleCircles[RIPPLE_COUNT];  /** 外周から中心へ収束する波紋の帯 */
			static constexpr float FIT_HALF_WIDTH = 6.0f;  /** 合わせ円の帯の半幅 */
			TerrainCircle m_fitCircles[static_cast<int>(EnFormationType::Num)];  /** 演出の大きさ合わせ用の円（陣形の色ごと） */
			int           m_fitCircleIndex = -1;          /** 描く合わせ円（-1で描かない） */
			float         m_rippleTimer = 0.0f;           /** 波紋アニメーションの経過時間（秒） */

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
