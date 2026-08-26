/**
 * @file ResultScene.cpp
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Menus/ResultMenu.h"
#include "Source/UI/UIParts.h" 
#include "TitleScene.h"


namespace
{
	constexpr float SCORE_BASE_MULTIPLIER = 100.0f; // 救出数の基本スコア倍率
	constexpr float SCENE_WAIT_TIME = 3.0f;   // 次シーンへの遷移待機秒数
	constexpr float SCORE_PER_ACHIEVEMENT = 2000.0f; // アチーブメント達成1件ごとの加算スコア

	// デバッグ用の自動プレイ（環境変数 BEAST_AUTOPLAY で有効）。
	// Aボタン入力なしでタイトルへ戻る
	bool IsAutoplayEnabled()
	{
		char buf[8];
		size_t len = 0;
		return getenv_s(&len, buf, sizeof(buf), "BEAST_AUTOPLAY") == 0 && len > 0 && buf[0] != '0';
	}
}


namespace app
{
	int ResultScene::s_collectedPenguin = 0;

	ResultScene::ResultScene()
		: m_collectedPenguin(0)
		, m_totalScore(0.0f)
		, m_resultMenu(nullptr)
	{}


	ResultScene::~ResultScene()
	{
		if (app::achievement::AchievementManager::GetInstance())
		{
			app::achievement::AchievementManager::DestroyInstance();
		}
	}


	bool ResultScene::Start()
	{
		m_collectedPenguin = s_collectedPenguin;

		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			m_allAchievementList = am->GetAllAchievements();
		}

		CalcTotalScore();

		// JSONから全ての静的UI（背景・テキスト・枠など）を読み込んで構築
		m_layout.Initialize<app::ui::ResultMenu>("Assets/parameter/result/result.json");
		m_resultMenu = m_layout.GetMenu<app::ui::ResultMenu>();

		if (m_resultMenu)
		{
			// Menuにスコアなどのデータを渡し、動的なUI（アチーブメント等）を生成させる
			m_resultMenu->SetResultData(m_collectedPenguin, m_totalScore, m_allAchievementList, SCORE_PER_ACHIEVEMENT);
		}

		SoundManager::Get().PlayBGM(enSoundKind_Result);

		return true;
	}


	void ResultScene::Update()
	{
		m_layout.Update();

		if (m_resultMenu && m_resultMenu->IsReadyToNextScene())
		{
			// 自動プレイ中はAボタン入力なしでタイトルへ戻る
			if (IsAutoplayEnabled() || g_pad[0]->IsTrigger(enButtonA))
			{
				SoundManager::Get().PlaySE(enSoundKind_ButtonEnter, 1.0f);
				m_nextScene = true;
			}
		}
	}


	void ResultScene::PauseUpdate()
	{}


	void ResultScene::Render(RenderContext& rc)
	{
		m_layout.Render(rc);
	}


	bool ResultScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene)
		{
			id = TitleScene::ID();
			waitTime = SCENE_WAIT_TIME;
			SoundManager::Get().StopBGM();
			return true;
		}
		return false;
	}


	void ResultScene::CalcTotalScore()
	{
		// アチーブメント達成数に応じてスコアを加算
		float achievementBonus = 0.0f;
		for (auto* achieve : m_allAchievementList)
		{
			if (achieve && achieve->IsAchieved())
			{
				achievementBonus += SCORE_PER_ACHIEVEMENT;
			}
		}

		float baseScore = static_cast<float>(m_collectedPenguin) * SCORE_BASE_MULTIPLIER;
		m_totalScore = baseScore + achievementBonus;

		/** ステージ別ハイスコアの更新（新記録ならファイルへ保存される）。
		 *  ステージ選択画面の「きろく」表示が周回の動機になる */
		ScoreManager::TryUpdateHighScore(static_cast<int>(m_totalScore));
	}
}