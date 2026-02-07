/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林尚哉
 */
#pragma once
#include "MyRenderer.h"


namespace nsBeastEngine
{
	class RenderingEngine : public Noncopyable
	{
	public:
		RenderingEngine();
		~RenderingEngine();


	public:

		/**
		 * @brief レンダリングパイプラインの初期化
		 * @param isSoftShadow	ソフトシャドウを使用するかどうか
		 */
		void Init(bool isSoftShadow);


	private:

		/**
		 * @brief メインレンダリングターゲットの初期化
		 */
		void InitMainRenderTarget();


	private:
		/**	メインレンダリングターゲット */
		RenderTarget m_mainRenderTarget;
	};
}