#include "stdafx.h"
#include "Application.h"
#include "Source/Scene/SceneManager.h"


namespace app
{
	Application::Application()
	{
		SceneManager::CreateInstance();
	}


	Application::~Application()
	{
		SceneManager::DestroyInstance();
	}


	void Application::Update()
	{
		SceneManager::GetInstance()->Update();
	}


	void Application::Render(RenderContext& rc)
	{
		SceneManager::GetInstance()->Render(rc);
	}
}