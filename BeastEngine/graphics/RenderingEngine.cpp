/**
 * @file RenderingEngine.cpp
 * @brief RenderingEngineクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "RenderingEngine.h"


namespace nsBeastEngine
{
	RenderingEngine::RenderingEngine()
	{
		g_sceneLight = &m_sceneLight;
	}


	RenderingEngine::~RenderingEngine()
	{
		g_sceneLight = nullptr;
	}


	void RenderingEngine::Init()
	{
		InitMainRenderTarget();
		InitCopyMainRenderTargetToFrameBufferSprite();
		// m_shadowMapRender.Init(); // ★一時コメントアウト
		// m_postEffect.Init(m_mainRenderTarget); // ★一時コメントアウト
		Init2DRenderTarget();
		m_sceneLight.Init();
	}


	void RenderingEngine::Update()
	{
		g_sceneLight->Update();
	}


	void RenderingEngine::InitMainRenderTarget()
	{
		float clearColor[4] = { 0.5f,0.5f,0.5f,1.0f };

		m_mainRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			clearColor
		);
	}


	void RenderingEngine::Init2DRenderTarget()
	{
		float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

		m_2DRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),  // ★UI_SPACE_WIDTHの代わり
			g_graphicsEngine->GetFrameBufferHeight(), // ★UI_SPACE_HEIGHTの代わり
			1,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			clearColor
		);

		nsK2EngineLow::SpriteInitData spriteInitData;

		spriteInitData.m_textures[0] = &m_2DRenderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = m_mainRenderTarget.GetWidth();
		spriteInitData.m_height = m_mainRenderTarget.GetHeight();
		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_vsEntryPointFunc = "VSMain";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		spriteInitData.m_alphaBlendMode = nsK2EngineLow::AlphaBlendMode_None;
		spriteInitData.m_colorBufferFormat[0] = m_mainRenderTarget.GetColorBufferFormat();
		m_2DSprite.Init(spriteInitData);

		spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = m_2DRenderTarget.GetWidth();
		spriteInitData.m_height = m_2DRenderTarget.GetHeight();
		m_mainSprite.Init(spriteInitData);
	}


	void RenderingEngine::InitShadowMapRender()
	{
		// m_shadowMapRender.Init(); // ★一時コメントアウト
	}


	void RenderingEngine::InitCopyMainRenderTargetToFrameBufferSprite()
	{
		nsK2EngineLow::SpriteInitData spriteInitData;

		spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();

		spriteInitData.m_width = g_graphicsEngine->GetFrameBufferWidth();
		spriteInitData.m_height = g_graphicsEngine->GetFrameBufferHeight();

		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		m_copyMainRtToFrameBufferSprite.Init(spriteInitData);
	}


	void RenderingEngine::CopyMainRenderTargetToFrameBufferSprite(nsK2EngineLow::RenderContext& rc)
	{
		// BeginGPUEvent("CopyMainRenderTargetToFrameBuffer"); // ★k2engine側でエラーが出る場合はコメントアウト

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

		// EndGPUEvent();
	}


	void RenderingEngine::Execute(nsK2EngineLow::RenderContext& rc)
	{
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		rc.ClearRenderTargetView(m_mainRenderTarget);
		RenderToShadowMap(rc);
		for (auto model : m_registerModels)
		{
			model->Draw(rc);
		}
		// m_postEffect.Render(rc, m_mainRenderTarget); // ★一時コメントアウト
		Render2D(rc);
		CopyMainRenderTargetToFrameBufferSprite(rc);
		m_registerModels.clear();
		m_renderObjects.clear();
	}


	void RenderingEngine::RenderToShadowMap(nsK2EngineLow::RenderContext& rc)
	{
		// BeginGPUEvent("RenderShadowmap");

		// m_shadowMapRender.Render(rc, m_renderObjects); // ★一時コメントアウト
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);

		// EndGPUEvent();
	}


	void RenderingEngine::Render2D(nsK2EngineLow::RenderContext& rc)
	{
		rc.WaitUntilToPossibleSetRenderTarget(m_2DRenderTarget);
		rc.SetRenderTargetAndViewport(m_2DRenderTarget);
		rc.ClearRenderTargetView(m_2DRenderTarget);
		m_mainSprite.Draw(rc);
		for (auto& renderObj : m_renderObjects)
		{
			renderObj->OnRender2D(rc);
		}
		rc.WaitUntilFinishDrawingToRenderTarget(m_2DRenderTarget);

		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		m_2DSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}
}