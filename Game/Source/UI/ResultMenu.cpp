/**
 * @file ResultMenu.cpp
 * @brief リザルトのUI動的処理・アニメーション管理クラス
 * @author 立山
 */
#include "stdafx.h"
#include "ResultMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/CRC32.h"



namespace
{
	// ---------------------------------------------------------
	// 色設定（フォントカラー）
	// ---------------------------------------------------------
	const Vector4 COLOR_DIGIT_TIME_SCORE = { 0.0f, 0.8f, 1.0f, 1.0f }; // シアン（例）
	const Vector4 COLOR_DIGIT_TOTAL = { 1.0f, 1.0f, 0.0f, 1.0f }; // 黄色

	// アニメーション用タイマー
	constexpr float CHECK_REVEAL_DELAY = 1.0f;
	constexpr float CHECK_REVEAL_INTERVAL = 0.5f;
	constexpr float TOTAL_REVEAL_DELAY = 0.5f;
	constexpr float TITLE_BUTTON_DELAY = 0.5f;

	// アチーブメント動的UI生成用のベース座標とオフセット（※動的生成のためここに定義）
	const Vector3 ACHIEVE_START_POS = { -200.0f, 150.0f, 0.0f };
	constexpr float ACHIEVE_OFFSET_X_CHECK = -70.0f;
	constexpr float ACHIEVE_OFFSET_X_NAME = 250.0f;
	constexpr float ACHIEVE_OFFSET_Y = -80.0f;

	constexpr float ACHIEVE_NAME_W = 570.0f;
	constexpr float ACHIEVE_NAME_H = 50.0f;
	constexpr float ACHIEVE_BOX_W = 40.0f;
	constexpr float ACHIEVE_BOX_H = 40.0f;
	constexpr float ACHIEVE_CHECK_W = 90.0f;
	constexpr float ACHIEVE_CHECK_H = 90.0f;

	// 合計スコア動的生成用
	constexpr float TOTAL_DIGIT_W = 80.0f;
	constexpr float TOTAL_DIGIT_H = 100.0f;
	constexpr float TOTAL_DIGIT_CENTER_X = 0.0f;
	constexpr float TOTAL_DIGIT_Y = -300.0f;
}



namespace app
{
	namespace ui
	{
		ResultMenu::ResultMenu()
			: m_clearTime(0.0f)
			, m_collectedPenguin(0)
			, m_totalScore(0.0f)
			, m_totalDigit(nullptr)
			, m_checkRevealTimer(0.0f)
			, m_checkRevealIndex(0)
			, m_allChecksRevealed(false)
			, m_postCheckTimer(0.0f)
			, m_totalScoreShown(false)
			, m_titleButtonShown(false)
		{}


		ResultMenu::~ResultMenu()
		{}


		void ResultMenu::InitializeLogic()
		{}


		void ResultMenu::SetResultData(float clearTime, int collectedPenguin, float totalScore, const std::vector<app::achievement::AchievementBase*>& achievements)
		{
			m_clearTime = clearTime;
			m_collectedPenguin = collectedPenguin;
			m_totalScore = totalScore;
			m_allAchievementList = achievements;

			auto* timeDigit = GetUI<UIDigit>(Hash32("ResultTimeDigit"));
			auto* scoreDigit = GetUI<UIDigit>(Hash32("ResultScoreDigit"));

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

			SetupAchievementUI();
		}


		void ResultMenu::Update()
		{
			auto* timeDigit = GetUI<UIDigit>(Hash32("ResultTimeDigit"));
			auto* scoreDigit = GetUI<UIDigit>(Hash32("ResultScoreDigit"));

			if (timeDigit)
			{
				timeDigit->m_color = COLOR_DIGIT_TIME_SCORE;
				timeDigit->SetNumber(static_cast<int>(m_clearTime));
			}
			if (scoreDigit)
			{
				scoreDigit->m_color = COLOR_DIGIT_TIME_SCORE;
				scoreDigit->SetNumber(m_collectedPenguin);
			}

			UpdateRevealSequence();

			MenuBase::Update();
		}


		void ResultMenu::UpdateRevealSequence()
		{
			const float dt = g_gameTime->GetFrameDeltaTime();

			// フェーズ1：チェックマークを順番に表示
			if (!m_allChecksRevealed)
			{
				if (m_checkRevealIndex < static_cast<int>(m_checkIconList.size()))
				{
					m_checkRevealTimer += dt;
					float threshold = CHECK_REVEAL_DELAY + (CHECK_REVEAL_INTERVAL * static_cast<float>(m_checkRevealIndex));

					if (m_checkRevealTimer >= threshold)
					{
						m_checkIconList[m_checkRevealIndex]->SetIsDraw(true);
						SoundManager::Get().PlaySE(enSoundKind_ResultCheck);
						m_checkRevealIndex++;
					}
				}
				else
				{
					m_allChecksRevealed = true;
					m_postCheckTimer = 0.0f;
				}
				return;
			}

			// フェーズ2：トータルスコアを表示
			if (!m_totalScoreShown)
			{
				m_postCheckTimer += dt;
				if (m_postCheckTimer >= TOTAL_REVEAL_DELAY)
				{
					auto* canvas = GetCanvas();
					if (canvas && !m_totalDigit)
					{
						uint32_t key = Hash32("TotalDigit");
						canvas->CreateUI<UIDigit>(key);
						m_totalDigit = canvas->FindUI<UIDigit>(key);
						if (m_totalDigit)
						{
							int score = static_cast<int>(m_totalScore);
							int digitCount = (score == 0) ? 1 : 0;
							int tmp = score;
							while (tmp > 0)
							{
								tmp /= 10;
								digitCount++;
							}

							float baseX = TOTAL_DIGIT_CENTER_X + static_cast<float>(digitCount - 1) * TOTAL_DIGIT_W / 2.0f;

							m_totalDigit->Initialize(
								"Assets/spriteData/UI/Number/White",
								digitCount, score,
								TOTAL_DIGIT_W, TOTAL_DIGIT_H,
								Vector3(baseX, TOTAL_DIGIT_Y, 0.0f),
								Vector3::One, Quaternion::Identity
							);
							m_totalDigit->m_color = COLOR_DIGIT_TOTAL;
							m_totalDigit->m_isDraw = true;
						}
					}
					m_totalScoreShown = true;
					m_postCheckTimer = 0.0f;
				}
				return;
			}

			// フェーズ3：Aボタンガイドを表示（JSONで用意したTitleBackTextのアルファを上げる）
			if (!m_titleButtonShown)
			{
				if (m_totalDigit)
				{
					m_totalDigit->m_color = COLOR_DIGIT_TOTAL;
					m_totalDigit->SetNumber(static_cast<int>(m_totalScore));
					m_totalDigit->m_isDraw = true;
				}

				m_postCheckTimer += dt;
				if (m_postCheckTimer >= TITLE_BUTTON_DELAY)
				{
					auto* titleBackText = GetUI<UIIcon>(Hash32("TitleBackText"));
					if (titleBackText)
					{
						// JSONで非表示(アルファ0)にしていたものを表示(アルファ1)にする
						titleBackText->m_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
					}
					m_titleButtonShown = true;
				}
			}
		}


		void ResultMenu::SetupAchievementUI()
		{
			auto* canvas = GetCanvas();
			if (!canvas) return;

			int rowIndex = 0;

			for (size_t i = 0; i < m_allAchievementList.size(); ++i)
			{
				auto* achieve = m_allAchievementList[i];
				if (!achieve) continue;

				float iconOffset = ACHIEVE_OFFSET_Y * static_cast<float>(rowIndex);
				float commonY = ACHIEVE_START_POS.y + iconOffset;

				Vector3 currentIconPos = ACHIEVE_START_POS;
				currentIconPos.x += ACHIEVE_OFFSET_X_CHECK;
				currentIconPos.y = commonY;

				Vector3 currentNamePos = ACHIEVE_START_POS;
				currentNamePos.x += ACHIEVE_OFFSET_X_NAME;
				currentNamePos.y = commonY;

				std::string nameAssetPath = "Assets/spriteData/UI/Achievement/AchieveName_/" + achieve->GetSpriteName() + ".DDS";
				std::string nameKeyName = "AchieveName_" + std::to_string(i);
				uint32_t nameKey = Hash32(nameKeyName.c_str());

				canvas->CreateUI<UIIcon>(nameKey);
				auto* nameIcon = canvas->FindUI<UIIcon>(nameKey);
				if (nameIcon)
				{
					nameIcon->Initialize(nameAssetPath.c_str(), ACHIEVE_NAME_W, ACHIEVE_NAME_H, currentNamePos, Vector3::One, Quaternion::Identity, Vector4::White);
					nameIcon->m_isDraw = true;
				}

				std::string checkBoxKeyName = "AchieveCheckBox_" + std::to_string(i);
				uint32_t checkBoxKey = Hash32(checkBoxKeyName.c_str());
				canvas->CreateUI<UIIcon>(checkBoxKey);
				auto* checkBoxIcon = canvas->FindUI<UIIcon>(checkBoxKey);
				if (checkBoxIcon)
				{
					checkBoxIcon->Initialize("Assets/spriteData/UI/Achievement/checkBox.DDS", ACHIEVE_BOX_W, ACHIEVE_BOX_H, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
					checkBoxIcon->m_isDraw = true;
				}

				if (achieve->IsAchieved())
				{
					std::string checkKeyName = "AchieveCheck_" + std::to_string(i);
					uint32_t checkKey = Hash32(checkKeyName.c_str());

					canvas->CreateUI<UIIcon>(checkKey);
					auto* checkIcon = canvas->FindUI<UIIcon>(checkKey);
					if (checkIcon)
					{
						checkIcon->Initialize("Assets/spriteData/UI/Achievement/check.DDS", ACHIEVE_CHECK_W, ACHIEVE_CHECK_H, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
						checkIcon->m_isDraw = false;
						m_checkIconList.push_back(checkIcon);
					}
				}
				rowIndex++;
			}
		}
	}
}