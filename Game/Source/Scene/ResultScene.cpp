/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/UIParts.h" 
#include "Source/Util/CRC32.h" 
#include "TitleScene.h"


namespace app
{
	float ResultScene::s_clearTime = 0.0f;
	int   ResultScene::s_collectedPenguin = 0;


	ResultScene::ResultScene()
		:m_clearTime(0.0f)
		, m_collectedPenguin(0)
		, m_totalScore(0.0f)
	{}


	ResultScene::~ResultScene()
	{
		if (app::achievement::AchievementManager::GetInstance()) {
			app::achievement::AchievementManager::DestroyInstance();
		}
	}


	bool ResultScene::Start()
	{
		m_resultRender.Init("Assets/spriteData/Scene/NorthPole.DDS", 1920.0f, 1080.0f);
		m_clearTimeRender.Init("Assets/spriteData/UI/TextSprite/Result/ClearTime.DDS", 480.0f, 270.0f);
		m_rescueRender.Init("Assets/spriteData/UI/TextSprite/Result/Rescue.DDS", 480.0f, 270.0f);
		m_titleBackRender.Init("Assets/spriteData/UI/TextSprite/Result/TitleBack.DDS", 480.0f, 270.0f);
		m_frame.Init("Assets/spriteData/UI/Frame/ResultFrame.DDS", 1920.0f, 1080.0f);

		m_clearTimeRender.SetPosition(Vector2(-300.0f, 300.0f));
		m_rescueRender.SetPosition(Vector2(300.0f, 300.0f));
		m_titleBackRender.SetPosition(Vector2(-450.0f, -375.0f));

		m_frame.SetScale(Vector2(1.2f, 1.5f));


		m_clearTime = s_clearTime;
		m_collectedPenguin = s_collectedPenguin;

		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			m_allAchievementList = am->GetAllAchievements();
		}

		CalcTotalScore();

		// JSONレイアウトを読み込んでUIを構築
		m_layout.Initialize<app::ui::MenuBase>("Assets/parameter/result/result.json");

		SetupAchievementUI();

		// 取得した値を UIDigit にセット
		auto* timeDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultTimeDigit"));
		auto* scoreDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultScoreDigit"));

		if (timeDigit)
		{
			timeDigit->SetNumber(static_cast<int>(m_clearTime));
			timeDigit->m_isDraw = true;
		}
		if (scoreDigit)
		{
			scoreDigit->SetNumber(m_collectedPenguin);
			scoreDigit->m_isDraw = true;
		}

		SoundManager::Get().PlayBGM(enSoundKind_Result);

		return true;
	}


	void ResultScene::Update()
	{
		m_layout.Update();   // UIの毎フレーム更新
		m_clearTimeRender.Update();
		m_rescueRender.Update();
		m_titleBackRender.Update();
		m_frame.Update();

		auto* timeDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultTimeDigit"));
		auto* scoreDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultScoreDigit"));
		auto* totalDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("TotalDigit"));

		if (timeDigit)  timeDigit->SetNumber(static_cast<int>(m_clearTime));
		if (scoreDigit) scoreDigit->SetNumber(m_collectedPenguin);
		if (totalDigit) totalDigit->SetNumber(m_totalScore);

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_nextScene = true;
		}
	}


	void ResultScene::PauseUpdate()
	{}


	void ResultScene::Render(RenderContext& rc)
	{
		m_resultRender.Draw(rc);
		m_frame.Draw(rc);
		m_rescueRender.Draw(rc);
		m_clearTimeRender.Draw(rc);
		m_titleBackRender.Draw(rc);
		m_layout.Render(rc); // UIの描画
	}


	bool ResultScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = TitleScene::ID();
			waitTime = 3.0f;
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
			// 達成済みのものだけカウントする
			if (achieve && achieve->IsAchieved())
			{
				achievedCount++;
			}
		}

		// ① アチーブメントの数による倍率（0個の場合は1倍にする）
		int achieveMultiplier = (achievedCount > 0) ? achievedCount : 1;

		// ② タイムボーナス倍率
		float timeMultiplier = 1.0f + (m_clearTime / 100.0f);

		// ③ 助けた子ペンギンの基本スコア
		float baseScore = static_cast<float>(m_collectedPenguin * 100);

		// ④ 最終計算
		m_totalScore = baseScore * achieveMultiplier * timeMultiplier;
	}


	void ResultScene::SetupAchievementUI()
	{
		auto* menu = m_layout.GetMenu<app::ui::MenuBase>();
		if (!menu) return;

		auto* canvas = menu->GetCanvas();
		if (!canvas) return;

		Vector3 startPos = { -200.0f, -150.0f, 0.0f };
		float offsetX = 100.0f;

		// 達成済みアイコンを詰めて描画するためのインデックス
		int drawIndex = 0;

		for (size_t i = 0; i < m_allAchievementList.size(); ++i)
		{
			auto* achieve = m_allAchievementList[i];

			if (achieve && achieve->IsAchieved())
			{
				std::string keyName = "AchieveIcon_" + std::to_string(i);
				uint32_t key = Hash32(keyName.c_str());

				canvas->CreateUI<app::ui::UIIcon>(key);
				auto* icon = canvas->FindUI<app::ui::UIIcon>(key);

				if (icon)
				{
					Vector3 pos = startPos;
					// i ではなく drawIndex を使うことで、隙間なくアイコンを並べる
					pos.x += offsetX * static_cast<float>(drawIndex);

					icon->Initialize(
						"Assets/spriteData/UI/Icon/DummyIcon.DDS",
						80.0f, 80.0f, // width, height
						pos,
						Vector3::One, // scale
						Quaternion::Identity, // rotation
						Vector4::White // color
					);

					icon->m_isDraw = true;

					drawIndex++; // 描画したらインデックスを進める
				}
			}
			else
			{
				// 未達成のアチーブメントに対する処理
				// ※もし「未達成の場合はシルエット（真っ黒の画像や半透明）を表示したい」
				// といった機能を追加したい場合は、ここに処理を書くことができます。
			}
		}
	}
}