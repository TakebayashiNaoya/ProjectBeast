/**
 * @file SubCameraManager.cpp
 * @brief サブカメラの管理を行うクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "SubCameraManager.h"
#include "CameraSystem.h"
#include "Graphics/RenderingEngine.h"


namespace nsBeastEngine
{
	namespace
	{
		/** サブカメラのRenderTarget幅 */
		constexpr UINT SUB_CAMERA_RT_WIDTH = 480;
		/** サブカメラのRenderTarget高さ */
		constexpr UINT SUB_CAMERA_RT_HEIGHT = 270;
		/** サブカメラのNearクリップ */
		constexpr float SUB_CAMERA_NEAR = 0.01f;
		/** サブカメラのFarクリップ */
		constexpr float SUB_CAMERA_FAR = 5000.0f;
	}


	SubCameraManager* SubCameraManager::m_instance = nullptr;


	void SubCameraManager::Begin(std::function<void()> onBegin)
	{
		if (m_isActive) return;

		CameraSystem::Get().CreateSubCamera();

		auto* subCamera = CameraSystem::Get().GetSubCamera();
		subCamera->SetNear(SUB_CAMERA_NEAR);
		subCamera->SetFar(SUB_CAMERA_FAR);

		InitRenderTargetAndSprite();

		m_isActive = true;

		if (onBegin)
		{
			onBegin();
		}
	}


	void SubCameraManager::End(std::function<void()> onEnd)
	{
		if (!m_isActive) return;

		m_isActive = false;

		CameraSystem::Get().DestroySubCamera();

		if (onEnd)
		{
			onEnd();
		}
	}


	void SubCameraManager::Update()
	{
		if (!m_isActive) return;

		UpdateSubCameraTransform();
	}


	void SubCameraManager::RenderOffscreen(nsK2EngineLow::RenderContext& rc)
	{
		if (!m_isActive) return;

		auto* subCamera = CameraSystem::Get().GetSubCamera();
		if (subCamera == nullptr) return;

		// サブカメラのビュープロジェクション行列からフラスタムを更新する
		Matrix viewProjMatrix;
		viewProjMatrix.Multiply(subCamera->GetViewMatrix(), subCamera->GetProjectionMatrix());
		m_frustum.Update(viewProjMatrix);

		// サブカメラ視点でオフスクリーンパスを実行する
		g_renderingEngine->RenderOffscreenPass(rc, *subCamera, m_frustum, m_renderTarget);
	}


	void SubCameraManager::RenderToScreen(nsK2EngineLow::RenderContext& rc)
	{
		if (!m_isActive)
		{
			return;
		}

		// TODO: 描画優先度を実装する
		m_sprite.Draw(rc);
	}


	void SubCameraManager::InitRenderTargetAndSprite()
	{
		m_renderTarget.Create(
			SUB_CAMERA_RT_WIDTH,
			SUB_CAMERA_RT_HEIGHT,
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D32_FLOAT
		);

		SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &m_renderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = SUB_CAMERA_RT_WIDTH;
		spriteInitData.m_height = SUB_CAMERA_RT_HEIGHT;
		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_psEntryPoinFunc = "PSMain";
		spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		m_sprite.Init(spriteInitData);
	}


	void SubCameraManager::UpdateSubCameraTransform()
	{
		auto* subCamera = CameraSystem::Get().GetSubCamera();
		if (subCamera == nullptr) return;

		// TODO: IsInDanger()が実装されたら危険な子ペンギンを候補に絞る
		// カメラ座標・ターゲット座標はGame側からSetCameraPosition()・SetTargetPosition()で設定する

		subCamera->SetPosition(m_cameraPosition);
		subCamera->SetTarget(m_targetPosition);
	}
}