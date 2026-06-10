/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#include "stdafx.h"
#include "TitleScene.h"

#include "DebugScene.h"
#include "NormalInGameScene.h"
#include "TutorialInGameScene.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TitleEventMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"
#include "Source/UI/StageSelect/StageSelectMenu.h"


namespace app
{
	TitleScene::TitleScene()
		: m_state(TitleState::Title)
		, m_nextScene(false)
		, m_titleEventPacket(nullptr)
		, m_soundOptionPacket(nullptr)
		, m_tutorialPacket(nullptr)
		, m_stageSelectPacket(nullptr)
	{}


	TitleScene::~TitleScene()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
	}


	bool TitleScene::Start()
	{
		ui::InitUIPacket(m_titleEventPacket, "Assets/parameter/title/Title.json");

		ui::InitUIPacket(m_soundOptionPacket, "Assets/parameter/sound/SoundOption.json");

		ui::InitUIPacket(m_tutorialPacket, "Assets/parameter/tutorial/Tutorial.json");

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
			if (m_titleEventPacket) { m_titleEventPacket->Render(rc); }
			break;

		case TitleState::StageSelect:
			if (m_stageSelectPacket) { m_stageSelectPacket->Render(rc); }
			break;

		case TitleState::SoundOption:
			if (m_soundOptionPacket) { m_soundOptionPacket->Render(rc); }
			break;

		case TitleState::Tutorial:
			if (m_tutorialPacket) { m_tutorialPacket->Render(rc); }
			break;

		default:
			break;
		}
	}

	bool TitleScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = m_nextSceneId;
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

			const uint32_t selectKey = m_titleEventPacket->GetMenu()->GetSelectKey();
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
				if (m_tutorialPacket->GetMenu())
				{
					m_tutorialPacket->GetMenu()->SetClosed(false);
				}
			}
			else if (selectKey == Hash32("EndIcon"))
			{
				// ゲームを終了する。
				PostQuitMessage(0);
				return;
			}
		}

		if (m_titleEventPacket)
		{
			m_titleEventPacket->Update();
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
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);

			// ステートを選択済みにする
			menu->SetIsSelected(true);

			switch (menu->GetSelectingStage())
			{
			case ui::EnStageChoices::Tutorial:
				m_nextSceneId = TutorialInGameScene::ID();
				break;
			case ui::EnStageChoices::Normal:
				m_nextSceneId = NormalInGameScene::ID();
				break;
			case ui::EnStageChoices::Easy:
			case ui::EnStageChoices::Hard:
				m_nextSceneId = NormalInGameScene::ID();
				break;
			default:
				m_nextSceneId = NormalInGameScene::ID();
				break;
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonB))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			// ステートを選択済みにする
			menu->Reset();
			m_state = TitleState::Title;
		}
	}


	void TitleScene::SoundOptionUpdate()
	{
		if (m_soundOptionPacket)
		{
			m_soundOptionPacket->Update();
		}

		if (g_pad[0]->IsTrigger(enButtonB))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_state = TitleState::Title;
		}
	}


	void TitleScene::TutorialUpdate()
	{
		if (m_tutorialPacket)
		{
			m_tutorialPacket->Update();
		}

		// TutorialMenu 内で Bボタンが押されたら閉じる。
		if (m_tutorialPacket->GetMenu() && m_tutorialPacket->GetMenu()->IsClosed())
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_tutorialPacket->GetMenu()->SetClosed(false);
			m_state = TitleState::Title;
		}
	}
}