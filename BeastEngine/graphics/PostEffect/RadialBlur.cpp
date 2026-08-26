/**
 * @file RadialBlur.cpp
 * @brief ラジアルブラーポストエフェクトクラスの実装
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/RadialBlur.h"


namespace nsBeastEngine
{
	namespace
	{
		/** ラジアルブラーシェーダーのファイルパス */
		constexpr const char* RADIAL_BLUR_FX_PATH = "Assets/shader/PostEffect/radialBlur.fx";

		/** メインRTへの書き戻しに使うシェーダーのファイルパス */
		constexpr const char* COPY_BACK_FX_PATH = "Assets/shader/sprite.fx";

		/**
		 * @brief 強度1.0のときのUV空間での最大サンプリング距離
		 * @details 0.08 は画面幅の8%ぶん画面端が流れる。それ以上は酔いやすい。
		 */
		constexpr float MAX_UV_DISTANCE = 0.08f;
	}


	void RadialBlur::Init(RenderTarget& mainRenderTarget)
	{
		// 書き戻し時のフォーマット変換を避けるため、メインRTと同じ設定で作成する
		// （フォーマットは直接引いてくる。ベタ書きするとメインRT側の変更に追従できない）
		const DXGI_FORMAT mainFormat = mainRenderTarget.GetColorBufferFormat();
		m_blurRenderTarget.Create(
			mainRenderTarget.GetWidth(),
			mainRenderTarget.GetHeight(),
			1,
			1,
			mainFormat,
			DXGI_FORMAT_UNKNOWN
		);

		// ブラー用スプライトを初期化する
		{
			SpriteInitData initData;
			initData.m_fxFilePath = RADIAL_BLUR_FX_PATH;
			initData.m_vsEntryPointFunc = "VSMain";
			initData.m_psEntryPoinFunc = "PSMain";
			initData.m_width = static_cast<UINT>(m_blurRenderTarget.GetWidth());
			initData.m_height = static_cast<UINT>(m_blurRenderTarget.GetHeight());
			initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
			initData.m_expandConstantBuffer = &m_cb;
			initData.m_expandConstantBufferSize = sizeof(m_cb);
			initData.m_alphaBlendMode = AlphaBlendMode_None;
			initData.m_colorBufferFormat[0] = mainFormat;

			m_blurSprite.Init(initData);
		}

		// 書き戻しは単純なコピーなので、既存のスプライトシェーダーを流用する
		{
			SpriteInitData initData;
			initData.m_fxFilePath = COPY_BACK_FX_PATH;
			initData.m_vsEntryPointFunc = "VSMain";
			initData.m_psEntryPoinFunc = "PSMain";
			initData.m_width = static_cast<UINT>(mainRenderTarget.GetWidth());
			initData.m_height = static_cast<UINT>(mainRenderTarget.GetHeight());
			initData.m_textures[0] = &m_blurRenderTarget.GetRenderTargetTexture();
			initData.m_alphaBlendMode = AlphaBlendMode_None;
			initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();

			m_copyBackSprite.Init(initData);
		}

		m_isInitialized = true;
	}


	void RadialBlur::Start(const float strength, const float attackTime, const float duration)
	{
		if (!m_isInitialized) return;
		if (strength <= 0.0f || duration <= 0.0f) return;

		// 発動中に呼ばれた場合は強い方を採用してやり直す
		if (m_duration > 0.0f && m_elapsedTime < m_duration && m_peakStrength > strength) return;

		m_peakStrength = min(1.0f, strength);
		m_attackTime = max(0.0f, min(attackTime, duration));
		m_duration = duration;
		m_elapsedTime = 0.0f;
	}


	void RadialBlur::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		if (!m_isInitialized || m_duration <= 0.0f) return;

		// ヒットストップ（ウルト発動・弾き反撃）と重なっても実時間どおりに終わらせる。
		// スケール後の時間で進めると、咆哮のブラーだけが間延びして見える
		m_elapsedTime += g_gameTime->GetUnscaledFrameDeltaTime();
		if (m_elapsedTime >= m_duration)
		{
			// 発動終了
			m_duration = 0.0f;
			return;
		}

		// エンベロープ：立ち上がりは2乗カーブで加速的に強くなり（咆哮の溜め）、
		// ピーク後は線形にゆっくり減衰する（叫びの余韻）。
		// 以前は残り時間の2乗で減衰させていたが、体感では指定時間の1/3ほどで
		// 見えなくなってしまったため線形にした
		float envelope = 0.0f;
		if (m_elapsedTime < m_attackTime)
		{
			const float t = m_elapsedTime / m_attackTime;
			envelope = t * t;
		}
		else
		{
			const float t = (m_elapsedTime - m_attackTime) / (m_duration - m_attackTime);
			envelope = 1.0f - t;
		}
		m_cb.strength = MAX_UV_DISTANCE * m_peakStrength * envelope;

		if (m_cb.strength <= 0.0f) return;

		// メインRTをブラーして専用RTへ書き込む
		rc.WaitUntilToPossibleSetRenderTarget(m_blurRenderTarget);
		rc.SetRenderTargetAndViewport(m_blurRenderTarget);
		m_blurSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_blurRenderTarget);

		// ブラー結果をメインRTへ書き戻す
		rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
		rc.SetRenderTargetAndViewport(mainRenderTarget);
		m_copyBackSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
	}

} // namespace nsBeastEngine
