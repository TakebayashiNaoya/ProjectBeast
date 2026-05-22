/**
 * @file Bloom.cpp
 * @brief ブルームポストエフェクトクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/Bloom.h"


namespace nsBeastEngine
{
	namespace
	{
		//============================================//
		// ブルームパラメーター
		// 調整する場合はここの値を変更する
		//============================================//

		/**
		 * @brief 輝度抽出のしきい値
		 * @details この値より暗いピクセルはブルームの対象外になる。
		 *          メインRTのフォーマットがR32G32B32A32_FLOATのため
		 *          1.0fを超えるHDR値も有効。
		 *          サンプルに合わせて1.0fを基準とする。
		 */
		constexpr float LUMINANCE_THRESHOLD = 1.5f;

		/** ブラーの強さ。数値が大きくなるほどボケが強くなる（ガウシアンブラー時のみ有効） */
		constexpr float BLUR_STRENGTH = 10.0f;

		/** ブルームの強度。メインRTへの加算合成時の明るさ倍率 */
		constexpr float BLOOM_INTENSITY = 2.0f;

		/**
		 * @brief 川瀬式ブルームの縮小バッファ数
		 * @details 1〜Bloom::MAX_KAWASE_BUFFERS の範囲で設定する。
		 *          大きいほどボケが広がるが処理負荷も増える。
		 *          kawaseBloom.fx は常に4枚参照するため、
		 *          未使用スロットは最後のバッファで埋める。
		 */
		constexpr int KAWASE_NUM_BUFFERS = 4;
	}


	void Bloom::Init(
		RenderTarget& mainRenderTarget,
		EnBloomType bloomType,
		EnBlurType blurType)
	{
		m_bloomType = bloomType;
		m_blurType = blurType;

		if (m_bloomType == EnBloomType::enNone)
		{
			// ブルームが無効の場合は初期化しない
			return;
		}

		// 縮小バッファ数を安全な範囲にクランプする
		m_numKawaseBuffers = max(1, min(KAWASE_NUM_BUFFERS, MAX_KAWASE_BUFFERS));

		// 輝度抽出用定数バッファデータを設定する
		m_luminanceCb.luminanceThreshold = LUMINANCE_THRESHOLD;

		// ブルーム合成用定数バッファデータを設定する
		m_bloomFinalCb.bloomIntensity = BLOOM_INTENSITY;

		// 各リソースを初期化する
		InitLuminanceRenderTarget(mainRenderTarget);
		InitLuminanceSprite(mainRenderTarget);

		const int rtWidth = mainRenderTarget.GetWidth();
		const int rtHeight = mainRenderTarget.GetHeight();

		if (m_bloomType == EnBloomType::enNormal)
		{
			// 通常ブルーム：輝度テクスチャに1段ブラーをかける
			// 解像度を半分に縮小してブラーをかける
			m_gaussianBlurs[0].Init(
				&m_luminanceRenderTarget.GetRenderTargetTexture(),
				m_blurType,
				rtWidth / 2,
				rtHeight / 2,
				BLUR_STRENGTH
			);
			InitNormalBloomFinalSprite(mainRenderTarget);
		}
		else
		{
			// 川瀬式ブルーム：前段のボケテクスチャを次段の入力として多段ブラーをかける
			// 各段で解像度を半分に縮小することでより広いボケを実現する
			// gaussianBlur[0]は輝度テクスチャにブラーをかける
			m_gaussianBlurs[0].Init(
				&m_luminanceRenderTarget.GetRenderTargetTexture(),
				m_blurType,
				rtWidth / 2,
				rtHeight / 2,
				BLUR_STRENGTH
			);
			// gaussianBlur[1]以降は前段のボケテクスチャを入力にする
			for (int i = 1; i < m_numKawaseBuffers; i++)
			{
				m_gaussianBlurs[i].Init(
					&m_gaussianBlurs[i - 1].GetBokeTexture(),
					m_blurType,
					rtWidth / (2 << i),
					rtHeight / (2 << i),
					BLUR_STRENGTH
				);
			}
			InitKawaseBloomFinalSprite(mainRenderTarget);
		}
	}


	void Bloom::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		if (m_bloomType == EnBloomType::enNone)
		{
			return;
		}

		// 輝度抽出
		RenderLuminance(rc);

		// ブラー
		RenderBlur(rc);

		// メインRTへの加算合成
		RenderFinal(rc, mainRenderTarget);
	}


	void Bloom::InitLuminanceRenderTarget(RenderTarget& mainRenderTarget)
	{
		m_luminanceRenderTarget.Create(
			mainRenderTarget.GetWidth(),
			mainRenderTarget.GetHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}


	void Bloom::InitLuminanceSprite(RenderTarget& mainRenderTarget)
	{
		SpriteInitData initData;
		initData.m_fxFilePath = "Assets/shader/PostEffect/bloom.fx";
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPoinFunc = "PSSamplingLuminance";
		initData.m_width = static_cast<UINT>(m_luminanceRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(m_luminanceRenderTarget.GetHeight());
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		initData.m_expandConstantBuffer = &m_luminanceCb;
		initData.m_expandConstantBufferSize = sizeof(m_luminanceCb);
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_luminanceSprite.Init(initData);
	}


	void Bloom::InitNormalBloomFinalSprite(RenderTarget& mainRenderTarget)
	{
		SpriteInitData initData;
		initData.m_fxFilePath = "Assets/shader/PostEffect/bloom.fx";
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPoinFunc = "PSBloomFinalNormal";
		initData.m_width = static_cast<UINT>(mainRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(mainRenderTarget.GetHeight());
		initData.m_textures[0] = &m_gaussianBlurs[0].GetBokeTexture();
		initData.m_expandConstantBuffer = &m_bloomFinalCb;
		initData.m_expandConstantBufferSize = sizeof(m_bloomFinalCb);
		initData.m_alphaBlendMode = AlphaBlendMode_Add;
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_finalSprite.Init(initData);
	}


	void Bloom::InitKawaseBloomFinalSprite(RenderTarget& mainRenderTarget)
	{
		SpriteInitData initData;
		initData.m_fxFilePath = "Assets/shader/PostEffect/kawaseBloom.fx";
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPoinFunc = "PSBloomFinalKawase";
		initData.m_width = static_cast<UINT>(mainRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(mainRenderTarget.GetHeight());

		// 使用する縮小バッファ分のボケテクスチャを設定する
		for (int i = 0; i < m_numKawaseBuffers; i++)
		{
			initData.m_textures[i] = &m_gaussianBlurs[i].GetBokeTexture();
		}

		// kawaseBloom.fx は t0〜t3 を常に参照するため、
		// 未使用スロットは最後のバッファを複製して埋め、未バインドSRV参照を防ぐ
		for (int i = m_numKawaseBuffers; i < MAX_KAWASE_BUFFERS; i++)
		{
			initData.m_textures[i] = &m_gaussianBlurs[m_numKawaseBuffers - 1].GetBokeTexture();
		}

		initData.m_expandConstantBuffer = &m_bloomFinalCb;
		initData.m_expandConstantBufferSize = sizeof(m_bloomFinalCb);
		initData.m_alphaBlendMode = AlphaBlendMode_Add;
		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_finalSprite.Init(initData);
	}


	void Bloom::RenderLuminance(RenderContext& rc)
	{
		rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRenderTarget);
		rc.SetRenderTargetAndViewport(m_luminanceRenderTarget);
		rc.ClearRenderTargetView(m_luminanceRenderTarget);
		m_luminanceSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_luminanceRenderTarget);
	}


	void Bloom::RenderBlur(RenderContext& rc)
	{
		const int numBlurs = (m_bloomType == EnBloomType::enNormal) ? 1 : m_numKawaseBuffers;
		for (int i = 0; i < numBlurs; i++)
		{
			m_gaussianBlurs[i].ExecuteOnGPU(rc);
		}
	}


	void Bloom::RenderFinal(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
		rc.SetRenderTargetAndViewport(mainRenderTarget);
		m_finalSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
	}

} // namespace nsBeastEngine