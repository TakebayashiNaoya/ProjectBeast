/**
 * @file DebugScene.h
 * @brief デバッグシーン
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "../../../BeastEngine/Resource/ResourceManager.h"
#include "../../../BeastEngine/Resource/ModelResource.h"
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
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			id = DebugScene::ID();
			return true;
		}
		return false;
	}
}