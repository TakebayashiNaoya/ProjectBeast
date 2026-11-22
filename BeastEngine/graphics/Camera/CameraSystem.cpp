/**
 * @file CameraSystem.cpp
 * @brief カメラシステムの実装
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

}