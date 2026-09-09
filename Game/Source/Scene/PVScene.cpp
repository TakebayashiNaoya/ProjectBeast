/**
 * @file PVScene.cpp
 * @brief PV（アトラクト映像）シーン
 */
#include "stdafx.h"
#include "PVScene.h"

#include "Source/Core/Fade.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/UIParts.h"
#include "TitleScene.h"


namespace
{
	/** レイアウト定義のパス */
	const char* LAYOUT_PATH = "Assets/parameter/pv/PV.json";
	/** タイトルへ戻るときのフェード時間（秒） */
	constexpr float FADE_TIME = 1.0f;

	// PVを中断する入力。押しっぱなしで抜けてしまわないようトリガー判定にする
	bool IsAnyButtonTriggered()
	{
		for (int i = 0; i < enButtonNum; i++)
		{
			if (g_pad[0]->IsTrigger(static_cast<EnButton>(i))) return true;
		}
		return false;
	}
}


namespace app
{
	PVScene::PVScene()
	{}


	PVScene::~PVScene()
	{
		// フェードアウトが終わった時点で破棄されるので、ここで音を確実に止める
		SoundManager::Get().StopBGM();
	}


	bool PVScene::Start()
	{
		m_layout.Initialize<ui::MenuBase>(LAYOUT_PATH);

		if (auto* menu = m_layout.GetMenu<ui::MenuBase>())
		{
			m_video = menu->GetUI<ui::UIVideo>(Hash32("PVVideo"));
		}

		// 映像を用意できなかったときは再生せずタイトルへ戻す
		if (m_video == nullptr)
		{
			K2_LOG("PVScene: 映像の取得に失敗しました: %s\n", LAYOUT_PATH);
			m_isReturnRequested = true;
		}

		return true;
	}


	void PVScene::Update()
	{
		m_layout.Update();

		if (m_isReturnRequested) return;

		if (!m_isPlaybackStarted)
		{
			// 暗転が明けてから再生を始める（フェード中に頭が流れてしまうのを防ぐ）
			if (core::Fade::Get().IsFading()) return;

			BeginPlayback();
			return;
		}

		// 再生し切ったか、何かボタンが押されたらタイトルへ戻る
		if (m_video->IsFinished() || IsAnyButtonTriggered())
		{
			SoundManager::Get().FadeOutBGM(FADE_TIME);
			m_isReturnRequested = true;
		}
	}


	void PVScene::PauseUpdate()
	{
		// フェードアウト中も映像の再生を続ける
		m_layout.Update();
	}


	void PVScene::Render(RenderContext& rc)
	{
		m_layout.Render(rc);
	}


	bool PVScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (!m_isReturnRequested) return false;

		id = TitleScene::ID();
		waitTime = FADE_TIME;
		return true;
	}


	void PVScene::BeginPlayback()
	{
		m_isPlaybackStarted = true;

		m_video->Play();
		SoundManager::Get().PlayBGM(enSoundKind_PV);
	}
}
