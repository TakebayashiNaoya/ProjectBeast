/**
 * @file PostEffectManager.cpp
 * @brief ポストエフェクトマネージャーの実装
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/PostEffectManager.h"


namespace nsBeastEngine
{
	void PostEffectManager::Init(
		RenderTarget& mainRenderTarget,
		EnBloomType bloomType,
		EnBlurType blurType,
		EnToneMapType toneMapType)
	{
		m_bloom.Init(mainRenderTarget, bloomType, blurType);
		m_radialBlur.Init(mainRenderTarget);
		m_toneMap.Init(mainRenderTarget, toneMapType);
	}


	void PostEffectManager::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		// HDRのまま輝度抽出を行う必要があるため、ブルームをトーンマップより先に実行する
		m_bloom.Render(rc, mainRenderTarget);

		// 咆哮などの衝撃演出。非発動中はコストゼロ
		m_radialBlur.Render(rc, mainRenderTarget);

		// 最後にLDRへ変換する
		m_toneMap.Render(rc, mainRenderTarget);
	}

} // namespace nsBeastEngine