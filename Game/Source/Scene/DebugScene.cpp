/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#if defined(APP_DEBUG)
#include "DebugScene.h"

#include "Source/UI/CountDownMenu.h"
#include "Source/UI/Layout.h"
#include "TitleScene.h"
#include "Source/Core/Fade.h"
#include "Source/UI/InGameTimerMenu.h"


namespace
{
	app::ui::Layout* layout = nullptr;
	app::ui::CountDownMenu* countDown = nullptr;

	app::ui::Layout* timerLayout = nullptr;
	app::ui::InGameTimerMenu* inGameTimer = nullptr;
}


namespace app
{
	DebugScene::DebugScene()
	{}


	DebugScene::~DebugScene()
	{}


	bool DebugScene::Start()
	{
		if (!layout)
		{
			layout = new ui::Layout();
			layout->Initialize<ui::CountDownMenu>
				("Assets/parameter/countDown/CountDown.json");
			countDown = layout->GetMenu<ui::CountDownMenu>();
		}
		if (!timerLayout)
		{
			const auto& fade = core::Fade::Get();
			timerLayout = new ui::Layout();
			timerLayout->Initialize<ui::InGameTimerMenu>
				("Assets/parameter/timer/InGameTimer.json");
			inGameTimer = timerLayout->GetMenu<ui::InGameTimerMenu>();
		}
		return true;
	}


	void DebugScene::Update()
	{
		const auto& fade = core::Fade::Get();
		if (countDown && inGameTimer)
		{
			if (!fade.IsFadeIn()
				&& !countDown->IsCountDownStart())
			{
				countDown->SetCountDownStartFlag(true);
			}
			if (!inGameTimer->IsTimerActive() 
				&& countDown->IsCountDownFinished())
			{
				//countDown->SetCountDownFinishedFlag(true);
				inGameTimer->StartTimer();
			}
		}
		if (layout)layout->Update();
		if (timerLayout)timerLayout->Update();
	}


	void DebugScene::PauseUpdate()
	{}


	void DebugScene::Render(RenderContext& rc)
	{
		if (layout)layout->Render(rc);
		if (timerLayout)timerLayout->Render(rc);
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			id = DebugScene::ID();
			return true;
		}
		if (g_pad[0]->IsTrigger(enButtonX))
		{
			id = TitleScene::ID();
			return true;
		}
		return false;
	}
}
#endif