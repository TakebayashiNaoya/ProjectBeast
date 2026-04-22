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





	// --------------------------------------------------
	// ゲージレンダー
	// --------------------------------------------------


	void GaugeRender::Update()
	{
		// 定数バッファをGPUに送る。
		UpdateConstantBuffer(
				&m_gaugeCb          // 送るデータ。
			,	sizeof(m_gaugeCb)   // サイズ。
			,	1                   // b1スロット。
		);
		m_sprite.Update(m_position, m_rotation, m_scale, m_pivot);
	}

	void GaugeRender::UpdateConstantBuffer(GaugeConstantBuffer* gaugeCb, size_t size, int slot)
	{
		m_sprite.GetExpandConstantBufferGPU();
	}


	void GaugeRender::Init(const char* filePath, float w, float h)
	{
		SpriteInitData initData;
		/** DDSのファイルの指定 */
		initData.m_ddsFilePath[0] = filePath;
		/** シェーダーのファイルパスの指定 */
		initData.m_fxFilePath = "Assets/shader/CircleGauge.fx";
		/** スプライトのサイズの指定 */
		initData.m_width = static_cast<UINT>(w);
		initData.m_height = static_cast<UINT>(h);

		/** ユーザー定義の拡張定数バッファの指定 */
		initData.m_expandConstantBuffer = &m_gaugeCb;
		/** ユーザー定義の拡張定数バッファのサイズの指定 */
		initData.m_expandConstantBufferSize = sizeof(GaugeConstantBuffer);
		m_sprite.Init(initData);
	}


	void GaugeRender::Draw(RenderContext& rc)
	{
		if (g_renderingEngine) {
			g_renderingEngine->AddRenderObject(this);
		}
	}
}