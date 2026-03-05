/**
 * @file IRenderer.h
 * @brief 描画されるオブジェクトが実装するインターフェース
 * @author 竹林尚哉
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
		 * @param rc レンダリングコンテキスト
		 */
		virtual void OnRenderShadowMap(RenderContext& rc) {}

		/**
		 * @brief 2D描画パスから呼ばれる処理
		 * @param rc レンダリングコンテキスト
		 */
		virtual void OnRender2D(RenderContext& rc) {}
	};
}
