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
		m_totalFrame.Init("Assets/spriteData/UI/Frame/ResultFrame.DDS", 480.0f, 1080.0f);

		m_clearTimeRender.SetPosition(Vector2(-300.0f, 400.0f));
		m_rescueRender.SetPosition(Vector2(300.0f, 400.0f));
		m_titleBackRender.SetPosition(Vector2(500.0f, -375.0f));

		m_frame.SetPosition(Vector2(0.0f, -10.0f));
		m_frame.SetScale(Vector2(1.2f, 2.0f));

		m_totalFrame.SetPosition(Vector2(0.0f, -500.0f));


		m_clearTime = s_clearTime;
		m_collectedPenguin = s_collectedPenguin;

		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			m_allAchievementList = am->GetAllAchievements();
		}
		// JSONレイアウトを読み込んでUIを構築
		m_layout.Initialize<app::ui::MenuBase>("Assets/parameter/result/result.json");

		SetupAchievementUI();

		CalcTotalScore();


		// 取得した値を UIDigit にセット
		auto* timeDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultTimeDigit"));
		auto* scoreDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultScoreDigit"));
		//auto* totalDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("TotalDigit"));

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
		//if (totalDigit)
		//{
		//	//totalDigit->SetNumber(static_cast<int>(m_totalScore));
		//	totalDigit->m_isDraw = false;
		//	totalDigit->m_color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
		//}

		// Aボタンガイドも最初は非表示（アルファ0で隠す）
		m_titleBackRender.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));


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
		//auto* totalDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("TotalDigit"));

		if (timeDigit)  timeDigit->SetNumber(static_cast<int>(m_clearTime));
		if (scoreDigit) scoreDigit->SetNumber(m_collectedPenguin);
		//if (totalDigit) totalDigit->SetNumber(m_totalScore);

		//if (m_checkRevealIndex < static_cast<int>(m_checkIconList.size()))
		//{
		//	m_checkRevealTimer += g_gameTime->GetFrameDeltaTime();

		//	// 最初の1つはDelay秒後、それ以降はInterval秒ごと
		//	float threshold = (m_checkRevealIndex == 0)
		//		? m_checkRevealDelay
		//		: m_checkRevealDelay + m_checkRevealInterval * m_checkRevealIndex;

		//	if (m_checkRevealTimer >= threshold)
		//	{
		//		m_checkIconList[m_checkRevealIndex]->m_isDraw = true;
		//		m_checkRevealIndex++;
		//	}
		//}

		UpdateRevealSequence();

		if (m_titleButtonShown && g_pad[0]->IsTrigger(enButtonA))
		{
			SoundManager::Get().PlaySE(enSoundKind_ButtonPush);
			m_nextScene = true;
		}
	}


	void ResultScene::UpdateRevealSequence()
	{
		const float dt = g_gameTime->GetFrameDeltaTime();

		// ── フェーズ1：チェックマークを1つずつ表示 ──────────────
		if (!m_allChecksRevealed)
		{
			if (m_checkRevealIndex < static_cast<int>(m_checkIconList.size()))
			{
				m_checkRevealTimer += dt;

				float threshold = m_checkRevealDelay
					+ m_checkRevealInterval * static_cast<float>(m_checkRevealIndex);

				if (m_checkRevealTimer >= threshold)
				{
					m_checkIconList[m_checkRevealIndex]->m_isDraw = true;
					SoundManager::Get().PlaySE(enSoundKind_ResultCheck);
					m_checkRevealIndex++;
				}
			}
			else
			{
				// 全チェック表示完了
				m_allChecksRevealed = true;
				m_postCheckTimer = 0.0f;
			}
			return; // フェーズ1中はここで終わり
		}

		// ── フェーズ2：スコアを表示 ──────────────────────────────
		if (!m_totalScoreShown)
		{
			m_postCheckTimer += dt;
			if (m_postCheckTimer >= m_totalRevealDelay)
			{
				// この時点で初めてUIDigitを動的生成する
				auto* canvas = m_layout.GetMenu<app::ui::MenuBase>()->GetCanvas();
				if (canvas && !m_totalDigit)
				{
					uint32_t key = Hash32("TotalDigit");
					canvas->CreateUI<app::ui::UIDigit>(key);
					m_totalDigit = canvas->FindUI<app::ui::UIDigit>(key);
					if (m_totalDigit)
					{
						// JSONで定義していた位置・サイズに合わせて調整してください
						m_totalDigit->Initialize(
							"Assets/spriteData/UI/Number",  // 数字画像のパス
							3,                             // 桁数
							static_cast<int>(m_totalScore),
							80.0f, 100.0f,                 // 幅・高さ
							Vector3(20.0f, -300.0f, 0.0f),  // 位置
							Vector3::One,
							Quaternion::Identity
						);
						m_totalDigit->m_isDraw = false;  // ← まず非表示のまま
						m_totalDigit->Update();          // ← 位置を即確定させる
						m_totalDigit->m_isDraw = true;   // ← その後に表示
					}
				}
				m_totalScoreShown = true;
				m_postCheckTimer = 0.0f;
			}
			return;
		}

		// ── フェーズ3：Aボタンガイドを表示 ──────────────────────
		if (!m_titleButtonShown)
		{
			// スコアの数値を毎フレーム維持（消えないように）
			if (m_totalDigit)
			{
				m_totalDigit->SetNumber(static_cast<int>(m_totalScore));
				m_totalDigit->m_isDraw = true;
			}

			m_postCheckTimer += dt;
			if (m_postCheckTimer >= m_titleButtonDelay)
			{
				m_titleBackRender.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
				m_titleButtonShown = true;
			}
		}
	}


	void ResultScene::PauseUpdate()
	{}


	void ResultScene::Render(RenderContext& rc)
	{
		m_resultRender.Draw(rc);
		m_frame.Draw(rc);
		//m_totalFrame.Draw(rc);
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

		//float rowOffsetY = -50.0f;
		Vector3 iconStartPos = { /*-500.0f, 90.0f, 0.0f*/-150.0f,200.0f,0.0f }; // 1個目の画像の位置（xで横、yで縦）
		float checkOffsetX = -70.0f;  // チェックアイコンのX位置
		float nameOffsetX = 250.0f;   // 名前画像のX位置

		float iconOffsetY = -80.0f;

		int rowIndex = 0;

		for (size_t i = 0; i < m_allAchievementList.size(); ++i)
		{
			auto* achieve = m_allAchievementList[i];
			if (!achieve)continue;

			float iconOffset = iconOffsetY * static_cast<float>(rowIndex);

			// 共通のY計算
			float commonY = iconStartPos.y + iconOffset;

			Vector3 currentIconPos = iconStartPos;
			currentIconPos.x += checkOffsetX;
			currentIconPos.y = commonY;

			Vector3 currentNamePos = iconStartPos;
			currentNamePos.x += nameOffsetX;
			currentNamePos.y = commonY; // 同じY値を使う

			std::string nameAssetPath = "Assets/spriteData/UI/Achievement/AchieveName_/"
				+ (achieve->GetSpriteName()) + ".DDS";

			std::string nameKeyName = "AchieveName_" + std::to_string(i);
			uint32_t nameKey = Hash32(nameKeyName.c_str());
			canvas->CreateUI<app::ui::UIIcon>(nameKey); // UITextではなくUIIconに変更
			auto* nameIcon = canvas->FindUI<app::ui::UIIcon>(nameKey);
			if (nameIcon)
			{
				nameIcon->Initialize(
					nameAssetPath.c_str(),
					570.0f, 50.0f, // 幅・高さは画像サイズに合わせて調整
					currentNamePos,
					Vector3::One,
					Quaternion::Identity,
					Vector4::White
				);
				nameIcon->m_isDraw = true;
			}


			std::string checkBoxKeyName = "AchieveCheckBox_" + std::to_string(i);
			uint32_t checkBoxKey = Hash32(checkBoxKeyName.c_str());

			canvas->CreateUI<app::ui::UIIcon>(checkBoxKey);
			auto* checkBoxIcon = canvas->FindUI<app::ui::UIIcon>(checkBoxKey);
			if (checkBoxIcon)
			{
				checkBoxIcon->Initialize(
					"Assets/spriteData/UI/Achievement/checkBox.DDS",
					40.0f, 40.0f, // width, height
					currentIconPos,
					Vector3::One, // scale
					Quaternion::Identity, // rotation
					Vector4::White // color
				);
				checkBoxIcon->m_isDraw = true;
			}


			achieve->SetIsAchieved(true);
			if (achieve->IsAchieved())
			{
				std::string checkKeyName = "AchieveCheck_" + std::to_string(i);
				uint32_t checkKey = Hash32(checkKeyName.c_str());

				canvas->CreateUI<app::ui::UIIcon>(checkKey);
				auto* checkIcon = canvas->FindUI<app::ui::UIIcon>(checkKey);

				if (checkIcon)
				{
					checkIcon->Initialize(
						"Assets/spriteData/UI/Achievement/check.DDS",
						90.0f, 90.0f, // width, height
						currentIconPos,
						Vector3::One, // scale
						Quaternion::Identity, // rotation
						Vector4::White // color
					);

					checkIcon->m_isDraw = false;
					m_checkIconList.push_back(checkIcon);
				}
			}
			else
			{
				// 未達成のアチーブメントに対する処理
				// ※もし「未達成の場合はシルエット（真っ黒の画像や半透明）を表示したい」
				// といった機能を追加したい場合は、ここに処理を書くことができます。
			}


			rowIndex++;
		}
	}
}