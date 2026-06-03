/**
 * @file RenderingEngine.cpp
 * @brief RenderingEngineクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "RenderingEngine.h"
#include "Camera/SubCameraManager.h"
#include "Camera/CameraSystem.h"


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

		// ポストエフェクトマネージャーの初期化
		InitPostEffectManager();

		m_sceneLight.Init();
	}


	void RenderingEngine::Update()
	{
		g_sceneLight->Update();
		SubCameraManager::Get().Update();
		CameraSystem::Get().Update();
	}


	void RenderingEngine::Execute(nsK2EngineLow::RenderContext& rc)
	{
		// サブカメラのオフスクリーン描画（メイン描画パスより前に実行する）
		SubCameraManager::Get().RenderOffscreen(rc);

		// ビュープロジェクション行列からフラスタム（視錐台）を更新する
		auto& mainCamera = CameraSystem::Get().GetMainCamera();
		Matrix viewProjMatrix;
		viewProjMatrix.Multiply(mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix());

#if defined(_DEBUG)
		// デバッグ時はフラスタムを画面内側に縮小して境界を画面上で確認できるようにする
		// 確認が終わったら Frustum.h の DEBUG_FRUSTUM_SHRINK_SCALE を 1.0f に戻すこと
		m_frustum.Update(viewProjMatrix, Frustum::DEBUG_FRUSTUM_SHRINK_SCALE);
#else
		m_frustum.Update(viewProjMatrix);
#endif

		// G-Bufferへの描画処理
		RenderToGBuffer(rc, mainCamera, m_frustum);

		// ディファードライティングの描画処理
		DeferredLighting(rc, m_mainRenderTarget);

		// フォワードレンダリングの描画処理
		ForwardRendering(rc, mainCamera, m_frustum, m_mainRenderTarget);

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

		// ポストエフェクトの描画処理
		// ※3D描画完了後・UI描画前に実行することでUIへの影響を防ぐ
		PostEffect(rc);

		// ブルーム完了後、mainRTをRTV状態に戻してエフェクトの描画先として設定する
		rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		rc.SetRenderTarget(
			m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
			m_gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		// エフェクトを描画
		// ※PostEffect()完了後に呼び出すことでブルームの影響を受けないようにする
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


	void RenderingEngine::RenderOffscreenPass(
		RenderContext& rc,
		nsK2EngineLow::Camera& camera,
		Frustum& frustum,
		RenderTarget& renderTarget)
	{
		RenderToGBuffer(rc, camera, frustum);
		DeferredLighting(rc, renderTarget);
		ForwardRendering(rc, camera, frustum, renderTarget);
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


	void RenderingEngine::InitPostEffectManager()
	{
		// ブルームの種別・ブラーの種別をここで切り替える
		m_postEffectManager.Init(
			m_mainRenderTarget,
			EnBloomType::enKawase,   // enNone / enNormal / enKawase
			EnBlurType::enGaussian   // enAverage / enGaussian
		);
	}


	void RenderingEngine::RenderToGBuffer(RenderContext& rc, nsK2EngineLow::Camera& camera, Frustum& frustum)
	{
		BeginGPUEvent("RenderToGBuffer");

		// レンダリングターゲットをG-Bufferに変更して書き込む
		RenderTarget* rts[] = {
			&m_gBuffer[enGBuffer_Albedo]    // 0番目のレンダリングターゲット
			,&m_gBuffer[enGBuffer_Normal]   // 1番目のレンダリングターゲット
			,&m_gBuffer[enGBuffer_Specular] // 2番目のレンダリングターゲット
		};

		// まず、レンダリングターゲットとして設定できるようになるまで待つ
		rc.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

		// レンダリングターゲットを設定
		rc.SetRenderTargets(ARRAYSIZE(rts), rts);

		// レンダリングターゲットをクリア
		rc.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

		// フラスタムカリングで視錐台内のモデルのみ描画する
		for (auto& MobjData : m_deferredModelList)
		{
			if (m_frustumCullingEnabled &&
				MobjData->IsCullingEnabled() &&
				!frustum.IsIntersectAABBWorld(MobjData->GetWorldAABBMin(), MobjData->GetWorldAABBMax()))
			{
				continue;
			}
			MobjData->OnDraw(rc);
		}

		// レンダリングターゲットへの書き込み待ち
		rc.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

		EndGPUEvent();
	}


	void RenderingEngine::DeferredLighting(RenderContext& rc, RenderTarget& renderTarget)
	{
		BeginGPUEvent("DeferredLighting");

		// レンダリング先を指定のレンダリングターゲットにする
		rc.WaitUntilToPossibleSetRenderTarget(renderTarget);
		rc.SetRenderTargetAndViewport(renderTarget);
		// G-Bufferの内容を元にしてディファードライティング
		m_deferredLightingSprite.Draw(rc);

		// レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(renderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::ForwardRendering(RenderContext& rc, nsK2EngineLow::Camera& camera, Frustum& frustum, RenderTarget& renderTarget)
	{
		BeginGPUEvent("ForwardRendering");

		// レンダリング先を指定のレンダリングターゲットにする
		rc.WaitUntilToPossibleSetRenderTarget(renderTarget);
		rc.SetRenderTarget(
			renderTarget.GetRTVCpuDescriptorHandle(),
			m_gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		// フラスタムカリングで視錐台内のモデルのみ描画する
		for (auto& renderObj : m_forwardModelList)
		{
			if (m_frustumCullingEnabled &&
				renderObj->IsCullingEnabled() &&
				!frustum.IsIntersectAABBWorld(renderObj->GetWorldAABBMin(), renderObj->GetWorldAABBMax()))
			{
				continue;
			}
			renderObj->OnDraw(rc);
		}

		// レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(renderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::PostEffect(RenderContext& rc)
	{
		BeginGPUEvent("PostEffect");

		m_postEffectManager.Render(rc, m_mainRenderTarget);

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

		// 小窓（サブカメラ）の描画
		SubCameraManager::Get().RenderToScreen(rc);

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