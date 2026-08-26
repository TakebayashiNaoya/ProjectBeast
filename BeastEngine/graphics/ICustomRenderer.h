/**
 * @file ICustomRenderer.h
 * @brief ゲーム側で自前のメッシュを描画するための基底インターフェース
 */
#pragma once
#include "BeastEnginePreCompile.h"
#include "Graphics/RenderViewContext.h"


namespace nsBeastEngine
{
	/**
	 * @brief カスタムメッシュ描画インターフェース
	 * @details ゲーム側で頂点バッファ・インデックスバッファを自前で組み立て、
	 *          独自シェーダーで描画したいオブジェクトが実装するインターフェース。
	 *          RenderingEngine::ForwardRendering() 内から呼ばれるため、
	 *          レンダーターゲット (DXGI_FORMAT_R32G32B32A32_FLOAT) と
	 *          深度バッファ (DXGI_FORMAT_D32_FLOAT) が設定済みの状態で呼び出される。
	 */
	class ICustomRenderer : public Noncopyable
	{
	public:
		virtual ~ICustomRenderer() = default;

		/**
		 * @brief 描画処理
		 * @param rc   レンダリングコンテキスト
		 * @param view 描画対象ビュー（カメラ・レンダーターゲットを含む）
		 */
		virtual void Render(RenderContext& rc, const RenderViewContext& view) = 0;
	};
}
