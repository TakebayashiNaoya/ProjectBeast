/**
 * @file GaussianBlur.cpp
 * @brief ガウシアンブラー・平均ブラーの独自実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/GaussianBlur.h"
#include <cmath>


namespace nsBeastEngine
{
	void GaussianBlur::Init(
		Texture* srcTexture,
		EnBlurType blurType,
		int blurWidth,
		int blurHeight,
		float sigma)
	{
		// 重みテーブルを計算する
		if (blurType == EnBlurType::enGaussian)
		{
			CalcWeightsTableFromGaussian(sigma);
		}
		else
		{
			SetAverageWeightsTable();
		}

		// 横ブラー用のレンダリングターゲットを作成する
		m_xBlurRenderTarget.Create(
			blurWidth,
			blurHeight,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);

		// 縦ブラー用のレンダリングターゲットを作成する
		m_yBlurRenderTarget.Create(
			blurWidth,
			blurHeight,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);

		// 各スプライトを初期化する
		InitXBlurSprite(srcTexture);
		InitYBlurSprite();
	}


	void GaussianBlur::ExecuteOnGPU(RenderContext& rc)
	{
		// 横ブラーを実行する
		rc.WaitUntilToPossibleSetRenderTarget(m_xBlurRenderTarget);
		rc.SetRenderTargetAndViewport(m_xBlurRenderTarget);
		rc.ClearRenderTargetView(m_xBlurRenderTarget);
		m_xBlurSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_xBlurRenderTarget);

		// 縦ブラーを実行する
		rc.WaitUntilToPossibleSetRenderTarget(m_yBlurRenderTarget);
		rc.SetRenderTargetAndViewport(m_yBlurRenderTarget);
		rc.ClearRenderTargetView(m_yBlurRenderTarget);
		m_yBlurSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_yBlurRenderTarget);
	}


	void GaussianBlur::CalcWeightsTableFromGaussian(float sigma)
	{
		float total = 0.0f;

		for (int x = 0; x < NUM_WEIGHTS; x++)
		{
			m_blurCb.weights[x] = std::expf(-0.5f * static_cast<float>(x * x) / sigma);
			total += 2.0f * m_blurCb.weights[x];
		}

		// 重みの合計が1になるよう正規化する
		for (int i = 0; i < NUM_WEIGHTS; i++)
		{
			m_blurCb.weights[i] /= total;
		}
	}


	void GaussianBlur::SetAverageWeightsTable()
	{
		// 全サンプルに均等な重みを設定する
		const float weight = 1.0f / static_cast<float>(NUM_WEIGHTS * 2);
		for (int i = 0; i < NUM_WEIGHTS; i++)
		{
			m_blurCb.weights[i] = weight;
		}
	}


	void GaussianBlur::InitXBlurSprite(Texture* srcTexture)
	{
		SpriteInitData initData;
		initData.m_fxFilePath = "Assets/shader/PostEffect/blur.fx";
		initData.m_vsEntryPointFunc = "VSXBlur";
		initData.m_psEntryPoinFunc = "PSBlur";
		initData.m_width = static_cast<UINT>(m_xBlurRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(m_xBlurRenderTarget.GetHeight());
		initData.m_textures[0] = srcTexture;
		initData.m_expandConstantBuffer = &m_blurCb;
		initData.m_expandConstantBufferSize = sizeof(m_blurCb);
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_xBlurSprite.Init(initData);
	}


	void GaussianBlur::InitYBlurSprite()
	{
		SpriteInitData initData;
		initData.m_fxFilePath = "Assets/shader/PostEffect/blur.fx";
		initData.m_vsEntryPointFunc = "VSYBlur";
		initData.m_psEntryPoinFunc = "PSBlur";
		initData.m_width = static_cast<UINT>(m_yBlurRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(m_yBlurRenderTarget.GetHeight());
		initData.m_textures[0] = &m_xBlurRenderTarget.GetRenderTargetTexture();
		initData.m_expandConstantBuffer = &m_blurCb;
		initData.m_expandConstantBufferSize = sizeof(m_blurCb);
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_yBlurSprite.Init(initData);
	}

} // namespace nsBeastEngine
