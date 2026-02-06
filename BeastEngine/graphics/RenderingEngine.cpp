#include "BeastEnginePreCompile.h"
#include "RenderingEngine.h"

namespace nsBeastEngine {

	RenderingEngine* RenderingEngine::m_instance = nullptr;

	RenderingEngine::RenderingEngine()
	{
		m_instance = this;
		// k2EngineLow側でg_sceneLightなどが参照されている場合に備えてグローバルポインタをつなぐ
		// (名前空間が異なるのでキャストが必要な場合がありますが、まずは自身のメンバを管理します)
	}

	RenderingEngine::~RenderingEngine()
	{
		m_instance = nullptr;
	}

	void RenderingEngine::Init(bool isSoftShadow)
	{
		m_isSoftShadow = isSoftShadow;

		InitMainRenderTarget();
		InitCopyMainRenderTargetToFrameBufferSprite();

		// ライトの初期化
		m_sceneLight.Init();

		// 他の初期化はファイルが揃うまで待機
		// InitZPrepassRenderTarget();
		// InitGBuffer();
		// InitShadowMapRender();
		// ...
	}

	void RenderingEngine::InitMainRenderTarget()
	{
		// HDRフォーマットで初期化
		m_mainRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);
	}

	void RenderingEngine::InitCopyMainRenderTargetToFrameBufferSprite()
	{
		SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = g_graphicsEngine->GetFrameBufferWidth();
		spriteInitData.m_height = g_graphicsEngine->GetFrameBufferHeight();
		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		m_copyMainRtToFrameBufferSprite.Init(spriteInitData);
	}

	void RenderingEngine::Update()
	{
		m_sceneLight.Update();
		// m_sceneGeometryData.Update(); // まだなし
	}

	void RenderingEngine::Execute(RenderContext& rc)
	{
		Update();

		// ライト情報を定数バッファ用構造体にコピー
		m_deferredLightingCB.m_light = m_sceneLight.GetSceneLight();
		m_deferredLightingCB.m_light.eyePos = g_camera3D->GetPosition();

		// --- 描画パス ---

		// 1. フォワードレンダリング (ここだけ生かす)
		ForwardRendering(rc);

		// 2. フレームバッファへのコピー
		CopyMainRenderTargetToFrameBuffer(rc);

		// リストクリア
		m_renderObjects.clear();
	}

	void RenderingEngine::ForwardRendering(RenderContext& rc)
	{
		// レンダリングターゲット設定待機
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);

		// RTとDSを設定
		rc.SetRenderTarget(
			m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
			m_mainRenderTarget.GetDSVCpuDescriptorHandle()
		);

		// クリア (本来はSkyBox描画などで埋めるが、まずは単色クリアで確認)
		float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		rc.ClearRenderTargetView(m_mainRenderTarget, clearColor);
		rc.ClearDepthStencilView(m_mainRenderTarget.GetDSVCpuDescriptorHandle(), 1.0f, 0);

		// 登録オブジェクトの描画
		for (auto& renderObj : m_renderObjects) {
			renderObj->OnForwardRender(rc);
		}

		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}

	void RenderingEngine::CopyMainRenderTargetToFrameBuffer(RenderContext& rc)
	{
		rc.SetRenderTarget(
			g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
			g_graphicsEngine->GetCurrentFrameBuffuerDSV()
		);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(g_graphicsEngine->GetFrameBufferWidth());
		viewport.Height = static_cast<FLOAT>(g_graphicsEngine->GetFrameBufferHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		rc.SetViewportAndScissor(viewport);

		m_copyMainRtToFrameBufferSprite.Draw(rc);
	}
}