/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#if defined(APP_DEBUG)
#include "DebugScene.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/Daddypenguin.h"
#include "Source/Core/ParameterManager.h"




namespace app
{
	DebugScene::DebugScene()
	{}


	DebugScene::~DebugScene()
	{}


	bool DebugScene::Start()
	{
		return true;
	}


	void DebugScene::Update()
	{}


	void DebugScene::PauseUpdate()
	{}


	void DebugScene::Render(RenderContext& rc)
	{}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}
#endif