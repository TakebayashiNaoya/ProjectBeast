/**
 * @file SpriteRender.cpp
 * @brief 2Dスプライト描画クラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "SpriteRender.h"


namespace nsBeastEngine
{
	void SpriteRender::Init(const char* filePath, float width, float height, AlphaBlendMode alphaBlendMode)
	{
		SpriteInitData initData;
		/** DDSのファイルパスの指定 */
		initData.m_ddsFilePath[0] = filePath;
		/** シェーダーのファイルパスの指定 */
		initData.m_fxFilePath = "Assets/shader/sprite.fx";
		/** スプライトのサイズの指定 */
		initData.m_width = static_cast<UINT>(width);
		initData.m_height = static_cast<UINT>(height);
		initData.m_alphaBlendMode = alphaBlendMode;

		m_sprite.Init(initData);
	}


	void SpriteRender::Draw(RenderContext& rc)
	{
		if (g_renderingEngine) {
			g_renderingEngine->AddRenderObject(this);
		}
	}
}