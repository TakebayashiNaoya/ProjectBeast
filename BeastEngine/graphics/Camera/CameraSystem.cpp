/**
 * @file CameraSystem.cpp
 * @brief カメラシステムの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "CameraSystem.h"


namespace nsBeastEngine
{
	CameraSystem* CameraSystem::m_instance = nullptr;


	void CameraSystem::Init()
	{
		// g_camera3D をメインカメラとしてラップする
		// 実体・更新は k2EngineLow（GraphicsEngine::BeginRender）が管理する
		m_mainCamera = g_camera3D;
		m_activeCamera = m_mainCamera;
	}


	void CameraSystem::Update()
	{
		// メインカメラは GraphicsEngine::BeginRender() が更新するため、ここでは不要
		if (m_subCamera != nullptr)
		{
			m_subCamera->Update();
		}
	}


	void CameraSystem::CreateSubCamera()
	{
		if (m_subCamera != nullptr)
		{
			K2_ASSERT(m_subCamera == nullptr, "サブカメラはすでに生成されています。");
			return;
		}
		m_subCamera = new nsK2EngineLow::Camera();
	}


	void CameraSystem::DestroySubCamera()
	{
		K2_ASSERT(m_subCamera != nullptr, "サブカメラはすでに破棄されています。");
		delete m_subCamera;
		m_subCamera = nullptr;
	}
}