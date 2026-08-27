/**
 * @file IRenderer.h
 * @brief 描画されるオブジェクトが実装するインターフェース
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief 描画されるオブジェクトが実装するインターフェース
	 * @details
	 *	描画されるオブジェクト（3Dモデルや2Dスプライト）が必ず継承すべき「共通のルール（基底クラス）」
	 *	影を描画する OnRenderShadowMap と、2Dを描画する OnRender2D という仮想関数を持っている
	 *	これにより、RenderingEngineは相手がモデルかスプライトかを気にせず、一括で処理できる（ポリモーフィズム）
	 */
	class IRenderer : public Noncopyable
	{
	public:
		/**
		 * @brief シャドウマップへの描画パスから呼ばれる処理
		 * @details カメラではなくライトから見た行列で描画するため、
		 *          ビュー行列とプロジェクション行列を受け取る。
		 * @details カスケードの番号も受け取る。
		 *          モデルの定数バッファはメッシュごとに1つしかないため、
		 *          1フレームに同じモデルを複数のカスケードへ描くと行列が上書きされてしまう。
		 *          カスケードごとに別のモデル（別の定数バッファ）を使う必要があり、
		 *          そのためにどのカスケードを描いているかを渡している。
		 * @param rc                レンダリングコンテキスト
		 * @param cascadeIndex      カスケードの番号（0が最も近景）
		 * @param lightViewMatrix   ライトのビュー行列
		 * @param lightProjMatrix   ライトのプロジェクション行列
		 */
		virtual void OnRenderShadowMap(
			nsK2EngineLow::RenderContext& rc,
			const int cascadeIndex,
			const nsK2EngineLow::Matrix& lightViewMatrix,
			const nsK2EngineLow::Matrix& lightProjMatrix) {}

		/**
		 * @brief 2D描画パスから呼ばれる処理
		 * @param rc レンダリングコンテキスト
		 */
		virtual void OnRender2D(nsK2EngineLow::RenderContext& rc) {}
	};
}