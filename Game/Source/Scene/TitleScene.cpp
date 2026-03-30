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


namespace app
{
	TitleScene::TitleScene()
		:m_state(TitleState::Title)
		, m_soundOption(nullptr)
		, m_soundOptionLayout(nullptr)
	{}


	TitleScene::~TitleScene()
	{
		delete m_soundOptionLayout;
	}


	bool TitleScene::Start()
	{
		m_titleRender.Init("Assets/spriteData/Scene/NorthPole.DDS", 1920.0f, 1080.0f);
		m_PenTakt.Init("Assets/spriteData/Scene/PenTakt.DDS", 480.0f, 270.0f);
		m_AButton.Init("Assets/spriteData/UI/Button/TitleScreen/Abutton.DDS", 480.0f, 270.0f);
		m_XButton.Init("Assets/spriteData/UI/Button/TitleScreen/Xbutton.DDS", 480.0f, 270.0f);
		m_PenTakt.SetPosition(Vector3(0.0f, 150.0f, 0.0f));
		m_PenTakt.SetScale(Vector2(2.5f, 2.5f));
		m_AButton.SetPosition(Vector2(-300.0f, -300.0f));
		m_XButton.SetPosition(Vector2(300.0f, -300.0f));
		m_soundOptionLayout = new ui::Layout;

		m_soundOptionLayout->Initialize<ui::SoundOptionMenu>(
			"Assets/parameter/sound/SoundOption.json"
		);

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
			if (g_pad[0]->IsTrigger(enButtonA))
			{
				SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
				SoundManager::Get().StopBGM();
				m_nextScene = true;
			}

			m_titleRender.Update();
			m_AButton.Update();
			m_XButton.Update();
			m_PenTakt.Update();

			if (g_pad[0]->IsTrigger(enButtonX))
			{
				SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
				m_state = TitleState::SoundOption;
				if (!m_soundOptionLayout)
				{
					m_soundOptionLayout = new ui::Layout;
					m_soundOptionLayout->Initialize<ui::SoundOptionMenu>(
						"Assets/parameter/sound/SoundOption.json"
					);

					m_soundOption = m_soundOptionLayout->GetMenu<ui::SoundOptionMenu>();
				}
			}
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
			m_AButton.Draw(rc);
			m_XButton.Draw(rc);
			m_PenTakt.Draw(rc);
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