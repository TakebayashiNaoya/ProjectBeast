#include "stdafx.h"
#include "Application.h"
#include "Source/Core/Fade.h"
#include "Source/Scene/SceneManager.h"


namespace app
{
	Application::Application()
	{
		core::Fade::Create();
		SceneManager::CreateInstance();
	}


	Application::~Application()
	{
		SceneManager::DestroyInstance();
		core::Fade::Delete();
	}


	void Application::Update()
	{
		SceneManager::GetInstance()->Update();
		core::Fade::Get().Update();
	}


	void Application::Render(RenderContext& rc)
	{
		SceneManager::GetInstance()->Render(rc);
		core::Fade::Get().Render(rc);
	}
}