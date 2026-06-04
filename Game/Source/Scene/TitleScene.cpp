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
#include "Source/UI/Menus/TutorialMenu.h"
#include "TitleScene.h"
#include "Source/UI/Menus/TitleEventMenu.h"


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

		ui::InitUIPacket(m_stageSelectPacket, "Assets/parameter/stageSelect/StageSelect.json");

		SoundManager::Get().PlayBGM(enSoundKind_Title);

		return true;
	}


	void TitleScene::Update()
	{
		switch (m_state)
		{
		case TitleState::Title:
		{
			if (!m_titleLayout)
			{
				m_titleLayout = new ui::Layout;
				m_titleLayout->Initialize<ui::TitleEventMenu>(
					"Assets/parameter/title/Title.json"
				);
				m_titleEventMenu = m_titleLayout->GetMenu<ui::TitleEventMenu>();
			}


			if (g_pad[0]->IsTrigger(enButtonA))
			{
				const uint32_t selectKey = m_titleEventMenu->GetSelectKey();
				if (selectKey == Hash32("StartFrameBackIcon"))
				{
					// ゲーム開始。
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					SoundManager::Get().StopBGM();
					m_nextScene = true;
				}
				else if (selectKey == Hash32("OptionFrameBackIcon"))
				{
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					m_state = TitleState::SoundOption;
					// オプション画面。
					if (!m_soundOption)
					{
						m_soundOptionLayout = new ui::Layout;
						m_soundOptionLayout->Initialize<ui::SoundOptionMenu>(
							"Assets/parameter/sound/SoundOption.json"
						);
						m_soundOption = m_soundOptionLayout->GetMenu<ui::SoundOptionMenu>();
					}
				}
				else if (selectKey == Hash32("RuleFrameBackIcon"))
				{
					// ルール画面。
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					m_state = TitleState::Tutorial;
					if (m_tutorialMenu)
					{
						m_tutorialMenu->SetClosed(false);
					}
				}
				else if (selectKey == Hash32("EndFrameBackIcon"))
				{
					// ゲームを終了する。
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					PostQuitMessage(0);
					return;
				}
			}

			m_titleRender.Update();
			m_titleLayout->Update();
			break;
		}

		case TitleState::SoundOption:
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
			break;
		}

		case TitleState::Tutorial:
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
			break;
		}
		}
	}

	void TitleScene::PauseUpdate()
	{}


	void TitleScene::Render(RenderContext& rc)
	{
		if (m_state == TitleState::Title)
		{
			if (m_titleLayout)
			{
				m_titleLayout->Render(rc);
			}
		}
		else if (m_state == TitleState::SoundOption)
		{
			if (m_soundOptionLayout)
			{
				m_soundOptionLayout->Render(rc);
			}
		}
		else if (m_state == TitleState::Tutorial)
		{
			if (m_tutorialLayout)
			{
				m_tutorialLayout->Render(rc);
			}
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
}