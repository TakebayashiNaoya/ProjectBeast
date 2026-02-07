/**
 * @file RenderingEngine.cpp
 * @brief RenderingEngineクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "RenderingEngine.h"


namespace nsBeastEngine
{
	RenderingEngine::RenderingEngine()
	{
	}


	RenderingEngine::~RenderingEngine()
	{
	}


	void RenderingEngine::Init(bool isSoftShadow)
	{
		/** メインレンダリングターゲットの初期化 */
		InitMainRenderTarget();
	}


	void RenderingEngine::InitMainRenderTarget()
	{
		m_mainRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			g_mainRenderTargetFormat.colorBufferFormat,
			g_mainRenderTargetFormat.depthBufferFormat
		);
	}
}