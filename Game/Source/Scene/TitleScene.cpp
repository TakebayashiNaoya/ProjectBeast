/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"
#include "InGameScene.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Layout.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TitleEventMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"
#include "Source/UI/StageSelect/StageSelectMenu.h"
#include "TitleScene.h"


namespace app
{
	TitleScene::TitleScene()
		:m_state(TitleState::Title)
		, m_soundOption(nullptr)
		, m_soundOptionLayout(nullptr)
		, m_tutorialLayout(nullptr)
		, m_tutorialMenu(nullptr)
		, m_titleEventMenu(nullptr)
		, m_titleLayout(nullptr)
		, m_stageSelectPacket(nullptr)
	{}


	TitleScene::~TitleScene()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
		delete m_soundOptionLayout;
		delete m_tutorialLayout;
		delete m_titleLayout;
	}


	bool TitleScene::Start()
	{
		m_titleLayout = new ui::Layout;
		m_titleLayout->Initialize<ui::TitleEventMenu>(
			"Assets/parameter/title/Title.json"
		);
		m_titleEventMenu = m_titleLayout->GetMenu<ui::TitleEventMenu>();


		m_soundOptionLayout = new ui::Layout;
		m_soundOptionLayout->Initialize<ui::SoundOptionMenu>(
			"Assets/parameter/sound/SoundOption.json"
		);
		m_soundOption = m_soundOptionLayout->GetMenu<ui::SoundOptionMenu>();

		m_tutorialLayout = new ui::Layout;
		m_tutorialLayout->Initialize<ui::TutorialMenu>(
			"Assets/parameter/tutorial/Tutorial.json"
		);
		m_tutorialMenu = m_tutorialLayout->GetMenu<ui::TutorialMenu>();

		ui::InitUIPacket(m_stageSelectPacket, "Assets/parameter/UI/stageSelect/StageSelect.json");

		SoundManager::Get().PlayBGM(enSoundKind_Title);

		return true;
	}


	void TitleScene::Update()
	{
		switch (m_state)
		{
		case TitleState::Title:
		{
			TitleUpdate();
			break;
		}

		case TitleState::StageSelect:
		{
			StageSelectUpdate();
			break;
		}

		case TitleState::SoundOption:
		{
			SoundOptionUpdate();
			break;
		}

		case TitleState::Tutorial:
		{
			TutorialUpdate();
			break;
		}
		}
	}

	void TitleScene::PauseUpdate()
	{}


	void TitleScene::Render(RenderContext& rc)
	{
		switch (m_state)
		{
		case TitleState::Title:
			if (m_titleLayout) { m_titleLayout->Render(rc); }
			break;

		case TitleState::StageSelect:
			if (m_stageSelectPacket) { m_stageSelectPacket->Render(rc); }
			break;

		case TitleState::SoundOption:
			if (m_soundOptionLayout) { m_soundOptionLayout->Render(rc); }
			break;

		case TitleState::Tutorial:
			if (m_tutorialLayout) { m_tutorialLayout->Render(rc); }
			break;

		default:
			break;
		}
	}

	bool TitleScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = InGameScene::ID();
			waitTime = 3.0f;
			return true;
		}
		return false;
	}


	void TitleScene::TitleUpdate()
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);

			const uint32_t selectKey = m_titleEventMenu->GetSelectKey();
			if (selectKey == Hash32("StartIcon"))
			{
				m_state = TitleState::StageSelect;
			}
			else if (selectKey == Hash32("SoundIcon"))
			{
				m_state = TitleState::SoundOption;
			}
			else if (selectKey == Hash32("RuleIcon"))
			{
				// ルール画面。
				m_state = TitleState::Tutorial;
				if (m_tutorialMenu)
				{
					m_tutorialMenu->SetClosed(false);
				}
			}
			else if (selectKey == Hash32("EndIcon"))
			{
				// ゲームを終了する。
				PostQuitMessage(0);
				return;
			}
		}

		if (m_titleLayout)
		{
			m_titleLayout->Update();
		}
	}


	void TitleScene::StageSelectUpdate()
	{
		if (!m_stageSelectPacket) return;

		m_stageSelectPacket->Update();

		auto* menu = m_stageSelectPacket->GetMenu();
		if (m_stageSelectPacket->GetMenu()->IsFinishedSelectAnimation())
		{
			menu->Reset();
			SoundManager::Get().StopBGM();
			m_nextScene = true;
			return;
		}

		// 選択済みなら抜ける
		if (menu->IsSelected()) return;
		// Aボタンが押されていなければ抜ける
		if (!g_pad[0]->IsTrigger(enButtonA)) return;

		SoundManager::Get().PlaySE(enSoundKind_ButtonPush);

		// ステートを選択済みにする
		menu->SetIsSelected(true);

		switch (menu->GetSelectingStage())
		{
		case ui::EnStageChoices::Back:
		{
			m_state = TitleState::Title;
			menu->Reset();
			break;
		}
		case ui::EnStageChoices::Easy:
		{
			break;
		}
		case ui::EnStageChoices::Normal:
		{
			break;
		}
		case ui::EnStageChoices::Hard:
		{
			break;
		}
		}
	}


	void TitleScene::SoundOptionUpdate()
	{
		if (m_soundOptionLayout)
		{
			m_soundOptionLayout->Update();
		}

		if (g_pad[0]->IsTrigger(enButtonB))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_state = TitleState::Title;
		}
	}


	void TitleScene::TutorialUpdate()
	{
		if (m_tutorialLayout)
		{
			m_tutorialLayout->Update();
		}

		// TutorialMenu 内で Bボタンが押されたら閉じる。
		if (m_tutorialMenu && m_tutorialMenu->IsClosed())
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_tutorialMenu->SetClosed(false);
			m_state = TitleState::Title;
		}
	}
}