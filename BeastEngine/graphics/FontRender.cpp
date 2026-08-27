/**
 * @file FontRender.cpp
 * @brief フォントレンダーの実装
 */
#include "BeastEnginePreCompile.h"
#include "FontRender.h"


namespace nsBeastEngine
{
	void FontRender::Draw(RenderContext& rc)
	{
		if (m_text == nullptr) return;

		g_renderingEngine->AddRenderObject(this);
	}
}