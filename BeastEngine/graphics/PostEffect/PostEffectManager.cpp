/**
 * @file PostEffectManager.cpp
 * @brief ポストエフェクトマネージャーの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/PostEffectManager.h"


namespace nsBeastEngine
{
	void PostEffectManager::Init(
		RenderTarget& mainRenderTarget,
		EnBloomType bloomType,
		EnBlurType blurType)
	{
		m_bloom.Init(mainRenderTarget, bloomType, blurType);
	}


	void PostEffectManager::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		m_bloom.Render(rc, mainRenderTarget);
	}

} // namespace nsBeastEngine