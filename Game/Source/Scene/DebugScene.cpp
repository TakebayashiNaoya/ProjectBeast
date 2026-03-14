/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "../../../BeastEngine/Resource/ResourceManager.h"
#include "../../../BeastEngine/Resource/ModelResource.h"
#include <time.h>


namespace app
{
	DebugScene::DebugScene()
	{}


	DebugScene::~DebugScene()
	{
		ResourceManager::GetInstance().Shutdown();
	}


	bool DebugScene::Start()
	{
		srand((unsigned int)time(NULL));
		ResourceManager::GetInstance().Start();
		ResourceManager::GetInstance().Register<ModelResource>(std::make_shared<ModelLoader>());
		ResourceManager::GetInstance().Register<AnimationResource>(std::make_shared<AnimationLoader>());

		for (int i = 0; i < 100; i++) {
			m_animationClip[i] = new AnimationClip;
			m_animationClip[i]->Load("Assets/animData/penguin/moveRun.tka");
			m_animationClip[i]->SetLoopFlag(true);
			m_modelRender[i].Init("Assets/modelData/penguin/daddyPenguin/DaddyPenguin.tkm", m_animationClip[i], 1);
			m_modelRender[i].SetPosition(rand() % 100, rand() % 100, rand() % 100);
			m_modelRender[i].Update();
		}

		return true;
	}


	void DebugScene::Update()
	{
		for (int i = 0; i < 100; i++)
		{
			m_modelRender[i].PlayAnimation(0);
			m_modelRender[i].Update();
		}
	}


	void DebugScene::Render(RenderContext& rc)
	{
		for (int i = 0; i < 100; i++)
		{
			m_modelRender[i].Draw(rc);
		}
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}