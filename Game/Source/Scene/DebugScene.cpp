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
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyControllerManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Stage/StageSystem.h"


namespace
{
	static app::actor::Enemy* enemy = nullptr;
	static app::actor::EnemyController* eneCon = nullptr;
}

namespace app
{
	DebugScene::DebugScene()
	{
		core::ParameterManager::CreateInstance();
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyControllerManager::CreateInstance();
	}


	DebugScene::~DebugScene()
	{
		core::ParameterManager::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		actor::EnemyControllerManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();
	}


	bool DebugScene::Start()
	{
		enemy = new actor::Enemy;
		eneCon = new actor::EnemyController;

		enemy->StartWrapper();
		eneCon->SetTarget(enemy);

		eneCon->AddTargetPos(Vector3(-700.0f, 0.0f, 500.0f));
		eneCon->AddTargetPos(Vector3(0.0f, 0.0f, 700.0f));
		eneCon->AddTargetPos(Vector3(0.0f, 0.0f, 100.0f));

		actor::ChildPenguinManager::GetInstance()->CreateChildPenguin(2);
		actor::ChildPenguinManager::GetInstance()->Start();
		return true;
	}


	void DebugScene::Update()
	{
		actor::StageSystem::GetInstance()->Update();
		enemy->UpdateWrapper();
		actor::EnemyControllerManager::GetInstance()->Update();

		actor::ChildPenguinManager::GetInstance()->Update();

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			eneCon->SetStun(true);
		}
	}


	void DebugScene::PauseUpdate()
	{}


	void DebugScene::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);
		enemy->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}
#endif