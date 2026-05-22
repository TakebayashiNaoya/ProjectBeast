/**
 * @file PostEffectManager.h
 * @brief ポストエフェクトマネージャー
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"
#include "Graphics/PostEffect/Bloom.h"


namespace nsBeastEngine
{
	/**
	 * @brief ポストエフェクトマネージャー
	 * @details RenderingEngine が保持し、Execute() 内で Render() を呼び出す。
	 *          3D描画完了後・UI描画前に実行することで、UIへの影響を防ぐ。
	 *          各ポストエフェクトをここで一括管理する。
	 *          将来の被写界深度・モーションブラー等の追加はここに行う。
	 */
	class PostEffectManager
	{
	public:
		PostEffectManager() = default;
		~PostEffectManager() = default;


	public:
		/**
		 * @brief 初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 * @param bloomType        ブルームの種別
		 * @param blurType         ブラーの種別
		 */
		void Init(
			RenderTarget& mainRenderTarget,
			EnBloomType bloomType,
			EnBlurType blurType
		);

		/**
		 * @brief 全ポストエフェクトを実行する
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Render(RenderContext& rc, RenderTarget& mainRenderTarget);


	private:
		/** ブルームエフェクト */
		Bloom m_bloom;
	};

} // namespace nsBeastEngine