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
#include "Source/UI/SoundOptionMenu.h"
#include "TitleScene.h"
#include "Source/UI/TitleEventMenu.h"


namespace app
{
	TitleScene::TitleScene()
		:m_state(TitleState::Title)
		, m_soundOption(nullptr)
		, m_soundOptionLayout(nullptr)
		, m_titleEventMenu(nullptr)
		, m_titleLayout(nullptr)
	{}


	TitleScene::~TitleScene()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
		delete m_soundOptionLayout;
		delete m_titleLayout;
	}


	bool TitleScene::Start()
	{
		m_titleRender.Init("Assets/spriteData/Scene/NorthPole.DDS", 1920.0f, 1080.0f);

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
		
		m_titleLayout->Reload();
		m_soundOptionLayout->Reload();

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
				if (selectKey == Hash32("StartHightLightIcon"))
				{
					// ゲーム開始。
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					SoundManager::Get().StopBGM();
					m_nextScene = true;
				}
				else if (selectKey == Hash32("OptionHightLightIcon"))
				{
					SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
					SoundManager::Get().StopBGM();
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
				else if (selectKey == Hash32("ReTitleHightLightIcon"))
				{
					// タイトルに。現在はタイトルから遷移しないため、ここにくることはない。
					K2_ASSERT(false, "警告です。");
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
		}
	}

	void TitleScene::PauseUpdate()
	{}


	void TitleScene::Render(RenderContext& rc)
	{
		if (m_state == TitleState::Title)
		{
			m_titleRender.Draw(rc);
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