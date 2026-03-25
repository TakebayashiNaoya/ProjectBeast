/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "../../../BeastEngine/Resource/ResourceManager.h"
#include "../../../BeastEngine/Resource/ModelResource.h"
#include "Source/Actor/Character/Player/Player.h"
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


	void DebugScene::Render(RenderContext& rc)
	{}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}