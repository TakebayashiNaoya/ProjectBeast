/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/Daddypenguin.h"
#include "Source/Core/ParameterManager.h"


static app::actor::DaddyPenguin* daddyPenguin = nullptr;


namespace app
{
	DebugScene::DebugScene()
	{
		core::ParameterManager::CreateInstance();
		daddyPenguin = new actor::DaddyPenguin();
	}


	DebugScene::~DebugScene()
	{}


	bool DebugScene::Start()
	{
		daddyPenguin->StartWrapper();
		return true;
	}


	void DebugScene::Update()
	{
		daddyPenguin->UpdateWrapper();
		core::ParameterManager::Get()->Update();
	}


	void DebugScene::Render(RenderContext& rc)
	{
		daddyPenguin->RenderWrapper(rc);
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}