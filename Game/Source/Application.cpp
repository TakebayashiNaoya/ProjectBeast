#include "stdafx.h"
#include "Application.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Core/Fade.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	Application::Application()
	{
		nsBeastEngine::nsCollision::PhysicsWorld::Initialize();
		camera::CameraManager::CreateInstance();
		core::ParameterManager::CreateInstance();
		core::Fade::Create();
		SoundManager::CreateInstance();
		SceneManager::CreateInstance();
		EffectManager::CreateInstance();

	}


	Application::~Application()
	{
		SceneManager::DestroyInstance();
		SoundManager::DestroyInstance();
		EffectManager::DestroyInstance();
		camera::CameraManager::DestroyInstance();
		core::ParameterManager::DestroyInstance();
		core::Fade::Delete();
		nsBeastEngine::nsCollision::PhysicsWorld::Finalize();
	}


	void Application::Update()
	{
		camera::CameraManager::Get().Update(6);
		core::ParameterManager::Get()->Update();
		SoundManager::Get().Update();
		SceneManager::GetInstance()->Update();
		EffectManager::Get().Update();
		core::Fade::Get().Update();
	}


	void Application::Render(RenderContext& rc)
	{
		SoundManager::Get().Update();
		SceneManager::GetInstance()->Render(rc);
		core::Fade::Get().Render(rc);
	}
}