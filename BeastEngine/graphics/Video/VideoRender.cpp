/**
 * @file VideoRender.cpp
 * @brief 映像を IRenderer として 2D パスで描画するクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "VideoRender.h"


namespace nsBeastEngine
{
	void VideoRender::Init(const char* clipPath, float dispWidth, float dispHeight, float fps)
	{
		if (!m_ownedClip.Load(clipPath))
		{
			K2_LOG("VideoRender::Init: クリップの読み込みに失敗しました: %s\n", clipPath);
			return;
		}
		m_ownedClip.SetFPS(fps);

		m_activeClip = &m_ownedClip;
		m_player.SetClip(m_activeClip);

		// 動的 GPU テクスチャを確保（内部で同期的な初回アップロードを実行）
		m_frameTex.Init(m_ownedClip.GetWidth(), m_ownedClip.GetHeight());
		if (!m_frameTex.IsInitialized())
		{
			K2_LOG("VideoRender::Init: VideoFrameTexture の初期化に失敗しました\n");
			return;
		}

		// UI 空間(UI_SPACE_WIDTH x UI_SPACE_HEIGHT)からビューポート空間(FRAME_BUFFER_W x FRAME_BUFFER_H)
		// へスケーリングする。これにより通常の SpriteRender と同じ座標感覚で使用できる。
		// 例: dispWidth=1920, dispHeight=1080 → 実スプライトサイズ 1600x900 → ちょうど画面いっぱい
		const float scaleX = static_cast<float>(FRAME_BUFFER_W) / static_cast<float>(UI_SPACE_WIDTH);
		const float scaleY = static_cast<float>(FRAME_BUFFER_H) / static_cast<float>(UI_SPACE_HEIGHT);

		SpriteInitData initData;
		initData.m_textures[0] = &m_frameTex.GetK2Texture();
		initData.m_fxFilePath = "Assets/shader/sprite.fx";
		initData.m_width = static_cast<UINT>(dispWidth * scaleX);
		initData.m_height = static_cast<UINT>(dispHeight * scaleY);
		initData.m_alphaBlendMode = AlphaBlendMode_Trans;
		m_sprite.Init(initData);

		m_isInitialized = true;
	}


	void VideoRender::ChangeClip(const char* clipPath, float fps)
	{
		if (!m_isInitialized) return;

		m_player.Stop();
		m_lastFrameIdx = -1;

		m_ownedClip.Unload();
		if (!m_ownedClip.Load(clipPath))
		{
			K2_LOG("VideoRender::ChangeClip: クリップの読み込みに失敗しました: %s\n", clipPath);
			return;
		}
		m_ownedClip.SetFPS(fps);
		m_activeClip = &m_ownedClip;
		m_player.SetClip(m_activeClip);
		m_player.Play();
	}


	void VideoRender::SwapClip(VideoClip* externalClip)
	{
		if (!m_isInitialized || !externalClip) return;

		m_player.Stop();
		m_lastFrameIdx = -1;
		m_activeClip = externalClip;
		m_player.SetClip(m_activeClip);
		m_player.Play();
	}


	void VideoRender::Update()
	{
		if (!m_isInitialized) return;

		m_player.Update(g_gameTime->GetFrameDeltaTime());
		m_sprite.SetMulColor(m_mulColor);
		m_sprite.Update(m_position, m_rotation, m_scale, m_pivot);
	}


	void VideoRender::Draw(RenderContext& /*rc*/)
	{
		if (!m_isInitialized) return;

		if (g_renderingEngine) {
			g_renderingEngine->AddRenderObject(this);
		}
	}


	void VideoRender::OnRender2D(RenderContext& rc)
	{
		if (!m_isInitialized || !m_activeClip) return;

		// フレームが変化した時だけ GPU テクスチャを更新する
		const int frameIdx = m_player.GetCurrentFrameIndex();
		if (frameIdx != m_lastFrameIdx)
		{
			const uint8_t* pixels = m_activeClip->GetFramePixels(frameIdx);
			if (pixels) m_frameTex.UploadFrame(pixels);
			m_lastFrameIdx = frameIdx;
		}

		m_sprite.Draw(rc);
	}
}
