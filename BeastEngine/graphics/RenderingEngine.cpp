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
	namespace
	{
		/** サブカメラのRenderTarget幅 */
		constexpr UINT SUB_CAMERA_RT_WIDTH = 480;
		/** サブカメラのRenderTarget高さ */
		constexpr UINT SUB_CAMERA_RT_HEIGHT = 270;
	}


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
		// メインビューの初期化
		m_mainView.width = g_graphicsEngine->GetFrameBufferWidth();
		m_mainView.height = g_graphicsEngine->GetFrameBufferHeight();
		m_mainView.camera = &CameraSystem::Get().GetMainCamera();
		m_mainView.renderTarget.Create(
			m_mainView.width,
			m_mainView.height,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);
		InitGBuffer(m_mainView);
		InitDeferredLightingSprite(m_mainView);

		// サブビューの初期化
		m_subView.width = SUB_CAMERA_RT_WIDTH;
		m_subView.height = SUB_CAMERA_RT_HEIGHT;
		m_subView.camera = CameraSystem::Get().GetSubCamera();
		m_subView.renderTarget.Create(
			m_subView.width,
			m_subView.height,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);
		InitGBuffer(m_subView);
		InitDeferredLightingSprite(m_subView);

		// メインレンダリングターゲットの内容をフレームバッファにコピーするためのスプライトの初期化
		InitCopyMainRenderTargetToFrameBufferSprite();

		// 2D描画用のレンダリングターゲットの初期化
		Init2DRenderTarget();

		// ポストエフェクトマネージャーの初期化
		InitPostEffectManager();

		// アクティブフラスタムのデフォルトをメインビューに設定する
		m_activeFrustum = &m_mainView.frustum;

		m_sceneLight.Init();
	}


	void RenderingEngine::Update()
	{
		g_sceneLight->Update();
	}


	void RenderingEngine::Execute(nsK2EngineLow::RenderContext& rc)
	{
		// メインカメラのフラスタムを更新する
		auto& mainCamera = CameraSystem::Get().GetMainCamera();
		m_mainView.camera = &mainCamera;
		Matrix viewProjMatrix;
		viewProjMatrix.Multiply(mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix());

#if defined(_DEBUG)
		// デバッグ時はフラスタムを画面内側に縮小して境界を画面上で確認できるようにする
		// 確認が終わったら Frustum.h の DEBUG_FRUSTUM_SHRINK_SCALE を 1.0f に戻すこと
		m_mainView.frustum.Update(viewProjMatrix, Frustum::DEBUG_FRUSTUM_SHRINK_SCALE);
#else
		m_mainView.frustum.Update(viewProjMatrix);
#endif

		// メインカメラの描画パスを実行する
		ExecuteViewPass(rc, m_mainView);

		// エフェクトの描画先としてmainRTを設定する
		// ※GBufferの深度を引き継ぐため、DSVはgBuffer[enGBuffer_Albedo]を使用する
		rc.WaitUntilToPossibleSetRenderTarget(m_mainView.renderTarget);
		rc.SetRenderTarget(
			m_mainView.renderTarget.GetRTVCpuDescriptorHandle(),
			m_mainView.gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		// エフェクトを描画
		// ※ポストエフェクトより前に描くことで、ブルームとトーンマップの対象に含める
		EffectEngine::GetInstance()->Draw();

		// エフェクト描画後もメインRTがレンダリングターゲット状態のままなので、
		// 以降のパスでテクスチャとして読めるように状態を戻す
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainView.renderTarget);

		// ポストエフェクトの描画処理（ブルーム → トーンマップ）
		// ※3D描画完了後・UI描画前に実行することでUIへの影響を防ぐ
		PostEffect(rc);

		// 2D描画処理（小窓スプライトの描画も含む）
		Render2D(rc);

		// サブカメラの描画パスを実行する（メイン描画パス完了後）
		if (CameraSystem::Get().HasSubCamera())
		{
			auto* subCamera = CameraSystem::Get().GetSubCamera();
			m_subView.camera = subCamera;
			Matrix subViewProjMatrix;
			subViewProjMatrix.Multiply(subCamera->GetViewMatrix(), subCamera->GetProjectionMatrix());
#if defined(_DEBUG)
			m_subView.frustum.Update(subViewProjMatrix, Frustum::DEBUG_FRUSTUM_SHRINK_SCALE);
#else
			m_subView.frustum.Update(subViewProjMatrix);
#endif

			// サブビューパスではもう一方のフレームバッファインデックスを使用する。
			// 同フレーム内でメインビューとサブビューが同じ定数バッファスロットに書き込むと
			// サブビューがメインビューのマトリクスを上書きしてしまうため、別スロットを使う。
			const UINT mainFrameIdx = g_graphicsEngine->GetBackBufferIndex();
			g_graphicsEngine->SetFrameIndex(1 - mainFrameIdx);

			ExecuteViewPass(rc, m_subView);

			g_graphicsEngine->SetFrameIndex(mainFrameIdx);
		}

		// メインレンダリングターゲットの内容をフレームバッファにコピー
		CopyMainRenderTargetToFrameBufferSprite(rc);

		// 全ビューの描画完了後、アクティブカメラをメインカメラに戻す
		// フレーム間でGetActiveCamera()が呼ばれた際にサブカメラが返らないようにするため
		CameraSystem::Get().SetActiveCamera(&CameraSystem::Get().GetMainCamera());
		m_activeFrustum = &m_mainView.frustum;

		// 描画オブジェクトのリストをクリア
		m_deferredModelList.clear();
		m_forwardModelList.clear();
		m_renderObjects.clear();
	}


	void RenderingEngine::ExecuteViewPass(RenderContext& rc, RenderViewContext& view)
	{
		CameraSystem::Get().SetActiveCamera(view.camera);
		m_activeFrustum = &view.frustum;

		RenderToGBuffer(rc, view);
		DeferredLighting(rc, view);
		ForwardRendering(rc, view);
		RenderNatureObjects(rc, view);
	}


	void RenderingEngine::InitGBuffer(RenderViewContext& view)
	{
		// アルベドカラー用のターゲットを作成
		view.gBuffer[enGBuffer_Albedo].Create(
			view.width,
			view.height,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);

		// 法線用のターゲットを作成
		view.gBuffer[enGBuffer_Normal].Create(
			view.width,
			view.height,
			1,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN
		);

		// PBRパラメータ用のターゲットを作成
		view.gBuffer[enGBuffer_Specular].Create(
			view.width,
			view.height,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}


	void RenderingEngine::InitDeferredLightingSprite(RenderViewContext& view)
	{
		SpriteInitData spriteInitData;
		spriteInitData.m_width = view.width;
		spriteInitData.m_height = view.height;
		spriteInitData.m_textures[enGBuffer_Albedo] = &view.gBuffer[enGBuffer_Albedo].GetRenderTargetTexture();
		spriteInitData.m_textures[enGBuffer_Normal] = &view.gBuffer[enGBuffer_Normal].GetRenderTargetTexture();
		spriteInitData.m_textures[enGBuffer_Specular] = &view.gBuffer[enGBuffer_Specular].GetRenderTargetTexture();
		spriteInitData.m_fxFilePath = "Assets/shader/DeferredLighting.fx";
		spriteInitData.m_expandConstantBuffer = m_sceneLight.GetLight();
		spriteInitData.m_expandConstantBufferSize = sizeof(Light);
		view.deferredLightingSprite.Init(spriteInitData);
	}


	void RenderingEngine::InitCopyMainRenderTargetToFrameBufferSprite()
	{
		nsK2EngineLow::SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &m_mainView.renderTarget.GetRenderTargetTexture();
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
		spriteInitData.m_width = m_mainView.renderTarget.GetWidth();
		spriteInitData.m_height = m_mainView.renderTarget.GetHeight();
		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_vsEntryPointFunc = "VSMain";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		spriteInitData.m_alphaBlendMode = AlphaBlendMode_None;
		spriteInitData.m_colorBufferFormat[0] = m_mainView.renderTarget.GetColorBufferFormat();
		m_2DSprite.Init(spriteInitData);

		spriteInitData.m_textures[0] = &m_mainView.renderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = m_2DRenderTarget.GetWidth();
		spriteInitData.m_height = m_2DRenderTarget.GetHeight();
		m_mainSprite.Init(spriteInitData);
	}


	void RenderingEngine::InitPostEffectManager()
	{
		// ブルーム・ブラー・トーンマップの種別をここで切り替える
		m_postEffectManager.Init(
			m_mainView.renderTarget,
			EnBloomType::enKawase,        // enNone / enNormal / enKawase
			EnBlurType::enGaussian,       // enAverage / enGaussian
			EnToneMapType::enReinhard     // enNone / enExposure / enReinhard /
										  // enReinhardExtended / enACES / enUncharted2
		);
	}


	void RenderingEngine::RenderToGBuffer(RenderContext& rc, RenderViewContext& view)
	{
		BeginGPUEvent("RenderToGBuffer");

		RenderTarget* rts[] = {
			&view.gBuffer[enGBuffer_Albedo]
			,&view.gBuffer[enGBuffer_Normal]
			,&view.gBuffer[enGBuffer_Specular]
		};

		rc.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);
		rc.SetRenderTargetsAndViewport(ARRAYSIZE(rts), rts);
		rc.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

		for (auto& MobjData : m_deferredModelList)
		{
			if (m_frustumCullingEnabled &&
				MobjData->IsCullingEnabled() &&
				!view.frustum.IsIntersectAABBWorld(MobjData->GetWorldAABBMin(), MobjData->GetWorldAABBMax()))
			{
				continue;
			}
			MobjData->OnDraw(rc);
		}

		rc.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

		EndGPUEvent();
	}


	void RenderingEngine::DeferredLighting(RenderContext& rc, RenderViewContext& view)
	{
		BeginGPUEvent("DeferredLighting");

		rc.WaitUntilToPossibleSetRenderTarget(view.renderTarget);
		rc.SetRenderTargetAndViewport(view.renderTarget);
		view.deferredLightingSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(view.renderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::ForwardRendering(RenderContext& rc, RenderViewContext& view)
	{
		BeginGPUEvent("ForwardRendering");

		rc.WaitUntilToPossibleSetRenderTarget(view.renderTarget);
		rc.SetRenderTarget(
			view.renderTarget.GetRTVCpuDescriptorHandle(),
			view.gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		for (auto& renderObj : m_forwardModelList)
		{
			if (m_frustumCullingEnabled &&
				renderObj->IsCullingEnabled() &&
				!view.frustum.IsIntersectAABBWorld(renderObj->GetWorldAABBMin(), renderObj->GetWorldAABBMax()))
			{
				continue;
			}
			renderObj->OnDraw(rc);
		}

		for (auto* renderer : m_customRenderers)
		{
			renderer->Render(rc, view);
		}

		rc.WaitUntilFinishDrawingToRenderTarget(view.renderTarget);

		EndGPUEvent();
	}


	void RenderingEngine::RenderNatureObjects(RenderContext& rc, RenderViewContext& view)
	{
		// GBufferに書き込まれた深度値を引き継ぐため、DSVはgBuffer[enGBuffer_Albedo]を使用する
		// ForwardRenderingと同じDSVを使うことで、ステージ・キャラクターとの深度関係が正しく保たれる
		rc.WaitUntilToPossibleSetRenderTarget(view.renderTarget);
		rc.SetRenderTarget(
			view.renderTarget.GetRTVCpuDescriptorHandle(),
			view.gBuffer[enGBuffer_Albedo].GetDSVCpuDescriptorHandle()
		);

		BeginGPUEvent("NatureObjects");
		for (auto* obj : m_natureObjects)
		{
			obj->Render(rc, view);
		}
		EndGPUEvent();

		rc.WaitUntilFinishDrawingToRenderTarget(view.renderTarget);
	}


	void RenderingEngine::PostEffect(RenderContext& rc)
	{
		BeginGPUEvent("PostEffect");

		m_postEffectManager.Render(rc, m_mainView.renderTarget);

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

		// 小窓（サブカメラ）の描画
		SubCameraManager::Get().RenderToScreen(rc);

		rc.WaitUntilFinishDrawingToRenderTarget(m_2DRenderTarget);

		rc.WaitUntilToPossibleSetRenderTarget(m_mainView.renderTarget);
		rc.SetRenderTargetAndViewport(m_mainView.renderTarget);
		m_2DSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_mainView.renderTarget);

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