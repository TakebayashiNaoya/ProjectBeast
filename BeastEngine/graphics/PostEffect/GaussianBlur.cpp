/**
 * @file GaussianBlur.cpp
 * @brief ガウシアンブラー・平均ブラーの独自実装
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

		// 一時バッファに重みを計算する
		float tmp[NUM_WEIGHTS];
		for (int x = 0; x < NUM_WEIGHTS; x++)
		{
			tmp[x] = std::expf(-0.5f * static_cast<float>(x * x) / sigma);

			if (x == 0)
			{
				// 中心テクセルは1回だけ加算する
				total += tmp[x];
			}
			else
			{
				// 中心以外は左右対称で2回分加算する
				// シェーダー側も±方向それぞれサンプルするため2回分として正規化する
				total += 2.0f * tmp[x];
			}
		}

		// 重みの合計が1になるよう正規化する
		for (int i = 0; i < NUM_WEIGHTS; i++)
		{
			tmp[i] /= total;
		}

		// Vector4[2]に詰め直す
		// weights[0].x    = 中心テクセルの重み
		// weights[0].yzw  = オフセット1〜3の重み
		// weights[1].xyzw = オフセット4〜7の重み
		m_blurCb.weights[0].x = tmp[0];
		m_blurCb.weights[0].y = tmp[1];
		m_blurCb.weights[0].z = tmp[2];
		m_blurCb.weights[0].w = tmp[3];
		m_blurCb.weights[1].x = tmp[4];
		m_blurCb.weights[1].y = tmp[5];
		m_blurCb.weights[1].z = tmp[6];
		m_blurCb.weights[1].w = tmp[7];
	}


	void GaussianBlur::SetAverageWeightsTable()
	{
		// blur.fx のPSは中心1回＋左右（上下）対称7サンプルの合計15サンプルを行う。
		// 合計が1になるよう 1/15 で均等な重みを設定する。
		// 中心(1回) + 左右7サンプル×2回 = 15サンプル
		const float weight = 1.0f / 15.0f;

		m_blurCb.weights[0] = Vector4(weight, weight, weight, weight);
		m_blurCb.weights[1] = Vector4(weight, weight, weight, weight);
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