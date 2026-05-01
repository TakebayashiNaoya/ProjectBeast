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
		// メインレンダリングターゲットの初期化
		InitMainRenderTarget();

		// Gバッファの初期化
		InitGBuffer();

		// ディファードライティング用のスプライトの初期化
		InitDeferredLightingSprite();

		// メインレンダリングターゲットの内容をフレームバッファにコピーするためのスプライトの初期化
		InitCopyMainRenderTargetToFrameBufferSprite();

		// 2D描画用のレンダリングターゲットの初期化
		Init2DRenderTarget();

		m_sceneLight.Init();
	}


	void RenderingEngine::Update()
	{
		g_sceneLight->Update();
	}


	void RenderingEngine::Execute(nsK2EngineLow::RenderContext& rc)
	{
		// G-Bufferへの描画処理
		RenderToGBuffer(rc);

		// ディファードライティングの描画処理
		DeferredLighting(rc);

		// フォワードレンダリングの描画処理
		ForwardRendering(rc);

		// 自然オブジェクトを描画する
		// GBufferに書き込まれた深度値を引き継ぐため、DSVはm_gBuffer[enGBuffer_Albedo]を使用する
		// ForwardRenderingと同じDSVを使うことで、ステージ・キャラクターとの深度関係が正しく保たれる
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTarget(
			m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
			m_gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		BeginGPUEvent("NatureObjects");
		for (auto* obj : m_natureObjects)
		{
			obj->Render(rc);
		}
		EndGPUEvent();

		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

		// エフェクトを描画
		EffectEngine::GetInstance()->Draw();

		// 2D描画処理
		Render2D(rc);

		// メインレンダリングターゲットの内容をフレームバッファにコピー
		CopyMainRenderTargetToFrameBufferSprite(rc);

		// 描画オブジェクトのリストをクリア
		m_deferredModelList.clear();
		m_forwardModelList.clear();
		m_renderObjects.clear();
	}


	void RenderingEngine::InitMainRenderTarget()
	{
		m_mainRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);
	}


	void RenderingEngine::InitGBuffer()
	{
		// アルベドカラー用のターゲットを作成
		m_gBuffer[enGBuffer_Albedo].Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);

		// 法線用のターゲットを作成
		m_gBuffer[enGBuffer_Normal].Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN
		);

		// PBRパラメータ用のターゲットを作成
		m_gBuffer[enGBuffer_Specular].Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}


	void RenderingEngine::InitDeferredLightingSprite()
	{
		// ディファードライティングを行うためのスプライトを初期化
		SpriteInitData spriteInitData;
		spriteInitData.m_width = FRAME_BUFFER_W;
		spriteInitData.m_height = FRAME_BUFFER_H;

		// ディファードライティングで使用するテクスチャを設定
		spriteInitData.m_textures[enGBuffer_Albedo] = &m_gBuffer[enGBuffer_Albedo].GetRenderTargetTexture();
		spriteInitData.m_textures[enGBuffer_Normal] = &m_gBuffer[enGBuffer_Normal].GetRenderTargetTexture();
		spriteInitData.m_textures[enGBuffer_Specular] = &m_gBuffer[enGBuffer_Specular].GetRenderTargetTexture();
		//spriteInitData.m_textures[enGBufferShadow] = &m_shadow.GetShadowTarget().GetRenderTargetTexture();

		spriteInitData.m_fxFilePath = "Assets/shader/DeferredLighting.fx";

		spriteInitData.m_expandConstantBuffer = m_sceneLight.GetLight();
		spriteInitData.m_expandConstantBufferSize = sizeof(Light);

		// ディファードレンダリング用のスプライトを初期化
		m_deferredLightingSprite.Init(spriteInitData);
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


	void RenderingEngine::Init2DRenderTarget()
	{
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		m_2DRenderTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
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
		spriteInitData.m_alphaBlendMode = AlphaBlendMode_None;
		spriteInitData.m_colorBufferFormat[0] = m_mainRenderTarget.GetColorBufferFormat();

		m_2DSprite.Init(spriteInitData);

		spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = m_2DRenderTarget.GetWidth();
		spriteInitData.m_height = m_2DRenderTarget.GetHeight();

		m_mainSprite.Init(spriteInitData);
	}


	void RenderingEngine::RenderToGBuffer(RenderContext& rc)
	{
		BeginGPUEvent("RenderToGBuffer");

		// レンダリングターゲットをG-Bufferに変更して書き込む
		RenderTarget* rts[] = {
			&m_gBuffer[enGBuffer_Albedo]   // 0番目のレンダリングターゲット
			,&m_gBuffer[enGBuffer_Normal]   // 1番目のレンダリングターゲット
			,&m_gBuffer[enGBuffer_Specular] // 2番目のレンダリングターゲット
		};

		// まず、レンダリングターゲットとして設定できるようになるまで待つ
		rc.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

		// レンダリングターゲットを設定
		rc.SetRenderTargets(ARRAYSIZE(rts), rts);

		// レンダリングターゲットをクリア
		rc.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

		// まとめてモデルレンダーを描画
		for (auto& MobjData : m_deferredModelList)
		{
			MobjData->OnDraw(rc);
		}

		// レンダリングターゲットへの書き込み待ち
		rc.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

		EndGPUEvent();
	}


	void RenderingEngine::DeferredLighting(RenderContext& rc)
	{
		BeginGPUEvent("DeferredLighting");

		// レンダリング先をメインレンダリングターゲットにする
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTargetAndViewport(m_mainRenderTarget);
		// G-Bufferの内容を元にしてディファードライティング
		m_deferredLightingSprite.Draw(rc);

		// メインレンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::ForwardRendering(RenderContext& rc)
	{
		BeginGPUEvent("ForwardRendering");

		// レンダリング先をメインレンダリングターゲットにする
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTarget(
			m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
			m_gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		// フォワードレンダリングで描画するオブジェクトを描画
		for (auto& renderObj : m_forwardModelList) {
			renderObj->OnDraw(rc);
		}

		// メインレンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::Render2D(nsK2EngineLow::RenderContext& rc)
	{
		BeginGPUEvent("Render2D");

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

		EndGPUEvent();
	}


	void RenderingEngine::CopyMainRenderTargetToFrameBufferSprite(nsK2EngineLow::RenderContext& rc)
	{
		BeginGPUEvent("CopyMainRenderTargetToFrameBuffer");

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

		EndGPUEvent();
	}
}