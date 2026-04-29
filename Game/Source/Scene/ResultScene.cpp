/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/ResultMenu.h"
#include "Source/Util/CRC32.h"
#include "TitleScene.h"



namespace
{
	constexpr float SCORE_TIME_DIVISOR = 100.0f; // タイムボーナス計算用の除数
	constexpr float SCORE_BASE_MULTIPLIER = 100.0f; // 救出数の基本スコア倍率
	constexpr float SCENE_WAIT_TIME = 3.0f;   // 次シーンへの遷移待機秒数
}



namespace app
{
	float ResultScene::s_clearTime = 0.0f;
	int   ResultScene::s_collectedPenguin = 0;


	ResultScene::ResultScene()
		: m_clearTime(0.0f)
		, m_collectedPenguin(0)
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
		m_clearTime = 65.0f;
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
			m_resultMenu->SetResultData(m_clearTime, m_collectedPenguin, m_totalScore, m_allAchievementList);
		}

		SoundManager::Get().PlayBGM(enSoundKind_Result);

		return true;
	}


	void ResultScene::Update()
	{
		m_layout.Update();

		if (m_resultMenu && m_resultMenu->IsReadyToNextScene())
		{
			if (g_pad[0]->IsTrigger(enButtonA))
			{
				SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
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
		int achievedCount = 0;
		for (auto* achieve : m_allAchievementList)
		{
			if (achieve && achieve->IsAchieved())
			{
				achievedCount++;
			}
		}

		int achieveMultiplier = (achievedCount > 0) ? achievedCount : 1;
		float timeMultiplier = 1.0f + (m_clearTime / SCORE_TIME_DIVISOR);
		float baseScore = static_cast<float>(m_collectedPenguin) * SCORE_BASE_MULTIPLIER;

		m_totalScore = baseScore * static_cast<float>(achieveMultiplier) * timeMultiplier;
	}
}