#include "stdafx.h"
#include "Application.h"
#include "Source/Core/Fade.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	Application::Application()
	{
		core::Fade::Create();
		SoundManager::CreateInstance();
		SceneManager::CreateInstance();
	}


	Application::~Application()
	{
		SceneManager::DestroyInstance();
		SoundManager::DestroyInstance();
		core::Fade::Delete();
	}


	void Application::Update()
	{
		SoundManager::Get().Update();
		SceneManager::GetInstance()->Update();
		core::Fade::Get().Update();
	}


	void Application::Render(RenderContext& rc)
	{
		SoundManager::Get().Update();
		SceneManager::GetInstance()->Render(rc);
		core::Fade::Get().Render(rc);
	}
}