/**
 * @file ResultMenu.cpp
 * @brief リザルトのUI動的処理・アニメーション管理クラス
 * @author 立山
 */
#include "stdafx.h"

#include "ResultMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"


namespace
{
	// ---------------------------------------------------------
	// 色設定（フォントカラー）
	// ---------------------------------------------------------
	const Vector4 COLOR_DIGIT_TIME_SCORE = { 0.0f, 0.3f, 1.0f, 1.0f }; // シアン（例）
	const Vector4 COLOR_DIGIT_TOTAL = { 1.0f, 1.0f, 0.0f, 1.0f }; // 黄色

	// アニメーション用タイマー
	constexpr float CHECK_REVEAL_DELAY = 1.0f;
	constexpr float CHECK_REVEAL_INTERVAL = 1.0f;
	constexpr float TOTAL_REVEAL_DELAY = 1.0f;
	constexpr float TITLE_BUTTON_DELAY = 1.5f;

	constexpr const char* RESULT_JSON_PATH = "Assets/parameter/result/result.json";
	constexpr const char* DYNAMIC_LAYOUT_KEY = "dynamic_layout";

	// ---------------------------------------------------------
	// 内部計算用定数（マジックナンバー排除用）
	// ---------------------------------------------------------
	constexpr int   SECONDS_PER_MINUTE = 60;   // 1分間の秒数
	constexpr int   SECONDS_DIGIT_COUNT = 2;    // 秒の表示桁数（常に2桁）
	constexpr int   DECIMAL_BASE = 10;   // 10進数計算の基数
	constexpr float CENTER_DIVISOR = 2.0f; // 中央揃え用の分割値
	constexpr float HALF_OFFSET_RATIO = 0.5f; // UI配置用の半幅オフセット

	int GetDigitCount(int number)
	{
		int digitCount = (number == 0) ? 1 : 0;
		int tmp = number;
		while (tmp > 0) { tmp /= DECIMAL_BASE; digitCount++; }
		return digitCount;
	}
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
			, m_drumRollHandle(app::INVALID_SE_HANDLE)
		{}


		ResultMenu::~ResultMenu()
		{}


		void ResultMenu::InitializeLogic()
		{
			if (!m_dataSet) return;

			// ホットリロード時：アニメーション状態をリセットして動的UIを再構築する
			m_checkRevealTimer = 0.0f;
			m_checkRevealIndex = 0;
			m_allChecksRevealed = false;
			m_postCheckTimer = 0.0f;
			m_totalScoreShown = false;
			m_titleButtonShown = false;
			m_totalDigit = nullptr;
			m_checkIconList.clear();

			ClearDynamicElements();
			BuildDynamicUI();
		}


		void ResultMenu::SetResultData(float clearTime, int collectedPenguin, float totalScore, const std::vector<app::achievement::AchievementBase*>& achievements)
		{
			m_clearTime = clearTime;
			m_collectedPenguin = collectedPenguin;
			m_totalScore = totalScore;
			m_allAchievementList = achievements;
			m_dataSet = true;

			ClearDynamicElements();
			BuildDynamicUI();

			m_drumRollHandle = SoundManager::Get().PlaySE(enSoundKind_DrumRoll, false);
		}


		void ResultMenu::ClearDynamicElements()
		{
			auto* canvas = GetCanvas();
			if (!canvas) return;

			canvas->RemoveUI(Hash32("ResultTimeMinDigit"));
			canvas->RemoveUI(Hash32("ResultTimeColonIcon"));
			canvas->RemoveUI(Hash32("ResultTimeSecDigit"));
			canvas->RemoveUI(Hash32("ResultScoreDigit"));
			canvas->RemoveUI(Hash32("TotalDigit"));

			for (size_t i = 0; i < m_allAchievementList.size(); ++i)
			{
				canvas->RemoveUI(Hash32(("AchieveBack_" + std::to_string(i)).c_str()));
				canvas->RemoveUI(Hash32(("AchieveName_" + std::to_string(i)).c_str()));
				canvas->RemoveUI(Hash32(("AchieveCheckBox_" + std::to_string(i)).c_str()));
				canvas->RemoveUI(Hash32(("AchieveCheck_" + std::to_string(i)).c_str()));
			}
		}


		void ResultMenu::LoadDynamicLayout()
		{
			nlohmann::json json;
			if (!app::util::JsonConverter::IsLoadJsonFile(json, RESULT_JSON_PATH)) return;
			if (!json.contains(DYNAMIC_LAYOUT_KEY)) return;

			const auto& dl = json[DYNAMIC_LAYOUT_KEY];
			using JC = app::util::JsonConverter;

			m_dynLayout.achieveStartPos = JC::ToVector3(dl, "achieve_start_pos", false, m_dynLayout.achieveStartPos);
			m_dynLayout.achieveOffsetY = JC::ToFloat(dl, "achieve_offset_y", m_dynLayout.achieveOffsetY);
			m_dynLayout.achieveOffsetXCheck = JC::ToFloat(dl, "achieve_offset_x_check", m_dynLayout.achieveOffsetXCheck);
			m_dynLayout.achieveOffsetXName = JC::ToFloat(dl, "achieve_offset_x_name", m_dynLayout.achieveOffsetXName);
			m_dynLayout.achieveOffsetXBack = JC::ToFloat(dl, "achieve_offset_x_back", m_dynLayout.achieveOffsetXBack);
			m_dynLayout.achieveNameFontSize = JC::ToFloat(dl, "achieve_name_font_size", m_dynLayout.achieveNameFontSize);
			m_dynLayout.achieveNamePivotX = JC::ToFloat(dl, "achieve_name_pivot_x", m_dynLayout.achieveNamePivotX);
			m_dynLayout.achieveBackW = JC::ToFloat(dl, "achieve_back_w", m_dynLayout.achieveBackW);
			m_dynLayout.achieveBackH = JC::ToFloat(dl, "achieve_back_h", m_dynLayout.achieveBackH);
			m_dynLayout.achieveBoxW = JC::ToFloat(dl, "achieve_box_w", m_dynLayout.achieveBoxW);
			m_dynLayout.achieveBoxH = JC::ToFloat(dl, "achieve_box_h", m_dynLayout.achieveBoxH);
			m_dynLayout.achieveCheckW = JC::ToFloat(dl, "achieve_check_w", m_dynLayout.achieveCheckW);
			m_dynLayout.achieveCheckH = JC::ToFloat(dl, "achieve_check_h", m_dynLayout.achieveCheckH);
			m_dynLayout.topDigitW = JC::ToFloat(dl, "top_digit_w", m_dynLayout.topDigitW);
			m_dynLayout.topDigitH = JC::ToFloat(dl, "top_digit_h", m_dynLayout.topDigitH);
			m_dynLayout.timeColonW = JC::ToFloat(dl, "time_colon_w", m_dynLayout.timeColonW);
			m_dynLayout.timeColonH = JC::ToFloat(dl, "time_colon_h", m_dynLayout.timeColonH);
			m_dynLayout.timeDigitCenterX = JC::ToFloat(dl, "time_digit_center_x", m_dynLayout.timeDigitCenterX);
			m_dynLayout.scoreDigitCenterX = JC::ToFloat(dl, "score_digit_center_x", m_dynLayout.scoreDigitCenterX);
			m_dynLayout.topDigitY = JC::ToFloat(dl, "top_digit_y", m_dynLayout.topDigitY);
			m_dynLayout.totalDigitW = JC::ToFloat(dl, "total_digit_w", m_dynLayout.totalDigitW);
			m_dynLayout.totalDigitH = JC::ToFloat(dl, "total_digit_h", m_dynLayout.totalDigitH);
			m_dynLayout.totalDigitCenterX = JC::ToFloat(dl, "total_digit_center_x", m_dynLayout.totalDigitCenterX);
			m_dynLayout.totalDigitY = JC::ToFloat(dl, "total_digit_y", m_dynLayout.totalDigitY);
		}


		void ResultMenu::BuildDynamicUI()
		{
			auto* canvas = GetCanvas();
			if (!canvas) return;

			LoadDynamicLayout();

			// タイトルへ戻るテキストは非表示にしておく
			auto* titleBackText = GetUI<UIIcon>(Hash32("TitleBackText"));
			if (titleBackText) titleBackText->m_isDraw = false;

			// ---------------------------------------------------------
			// クリアタイムの動的生成（M:SS形式）
			// ---------------------------------------------------------
			int totalSec = static_cast<int>(m_clearTime);
			int minutes = totalSec / SECONDS_PER_MINUTE;
			int seconds = totalSec % SECONDS_PER_MINUTE;

			int minutesDigitCount = GetDigitCount(minutes);

			const auto& L = m_dynLayout;
			float totalW = (minutesDigitCount * L.topDigitW) + L.timeColonW + (SECONDS_DIGIT_COUNT * L.topDigitW);
			float leftX = L.timeDigitCenterX - (totalW / CENTER_DIVISOR);
			float minutesBaseX = leftX + (minutesDigitCount - HALF_OFFSET_RATIO) * L.topDigitW;
			float colonCenterX = leftX + (minutesDigitCount * L.topDigitW) + (L.timeColonW / CENTER_DIVISOR);
			float secondsBaseX = leftX + totalW - (L.topDigitW / CENTER_DIVISOR);

			uint32_t minKey = Hash32("ResultTimeMinDigit");
			canvas->CreateUI<UIDigit>(minKey);
			auto* minDigit = canvas->FindUI<UIDigit>(minKey);
			if (minDigit)
			{
				minDigit->Initialize("Assets/spriteData/UI/Number/White", minutesDigitCount, minutes, L.topDigitW, L.topDigitH, Vector3(minutesBaseX, L.topDigitY, 0.0f), Vector3::One, Quaternion::Identity);
				minDigit->m_isDraw = true;
			}

			uint32_t colonKey = Hash32("ResultTimeColonIcon");
			canvas->CreateUI<UIIcon>(colonKey);
			auto* colonIcon = canvas->FindUI<UIIcon>(colonKey);
			if (colonIcon)
			{
				colonIcon->Initialize("Assets/spriteData/UI/Icon/InGameTimerIcon/Clone.dds", L.timeColonW, L.timeColonH, Vector3(colonCenterX, L.topDigitY, 0.0f), Vector3::One, Quaternion::Identity, Vector4::White);
				colonIcon->m_isDraw = true;
			}

			uint32_t secKey = Hash32("ResultTimeSecDigit");
			canvas->CreateUI<UIDigit>(secKey);
			auto* secDigit = canvas->FindUI<UIDigit>(secKey);
			if (secDigit)
			{
				secDigit->Initialize("Assets/spriteData/UI/Number/White", SECONDS_DIGIT_COUNT, seconds, L.topDigitW, L.topDigitH, Vector3(secondsBaseX, L.topDigitY, 0.0f), Vector3::One, Quaternion::Identity);
				secDigit->m_isDraw = true;
			}

			// ---------------------------------------------------------
			// 助けた数の動的生成
			// ---------------------------------------------------------
			uint32_t scoreKey = Hash32("ResultScoreDigit");
			canvas->CreateUI<UIDigit>(scoreKey);
			auto* scoreDigit = canvas->FindUI<UIDigit>(scoreKey);
			if (scoreDigit)
			{
				int digitCount = GetDigitCount(m_collectedPenguin);
				float baseX = L.scoreDigitCenterX + static_cast<float>(digitCount - 1) * L.topDigitW / CENTER_DIVISOR;
				scoreDigit->Initialize("Assets/spriteData/UI/Number/White", digitCount, m_collectedPenguin, L.topDigitW, L.topDigitH, Vector3(baseX, L.topDigitY, 0.0f), Vector3::One, Quaternion::Identity);
				scoreDigit->m_isDraw = true;
			}

			SetupAchievementUI();
		}


		void ResultMenu::Update()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				// 変更箇所：分割した3つのパーツと助けた数を取得
				auto* minDigit = canvas->FindUI<UIDigit>(Hash32("ResultTimeMinDigit"));
				auto* colonIcon = canvas->FindUI<UIIcon>(Hash32("ResultTimeColonIcon"));
				auto* secDigit = canvas->FindUI<UIDigit>(Hash32("ResultTimeSecDigit"));

				auto* scoreDigit = canvas->FindUI<UIDigit>(Hash32("ResultScoreDigit"));

				int totalSec = static_cast<int>(m_clearTime);

				if (minDigit)
				{
					minDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					minDigit->SetNumber(totalSec / SECONDS_PER_MINUTE);
				}
				if (colonIcon)
				{
					colonIcon->m_color = COLOR_DIGIT_TIME_SCORE;
				}
				if (secDigit)
				{
					secDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					secDigit->SetNumber(totalSec % SECONDS_PER_MINUTE);
				}

				if (scoreDigit)
				{
					scoreDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					scoreDigit->SetNumber(m_collectedPenguin);
				}
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
						SoundManager::Get().PlaySE(enSoundKind_Stamp);
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
								tmp /= DECIMAL_BASE;
								digitCount++;
							}

							const auto& L = m_dynLayout;
							float baseX = L.totalDigitCenterX + static_cast<float>(digitCount - 1) * L.totalDigitW / CENTER_DIVISOR;

							m_totalDigit->Initialize(
								"Assets/spriteData/UI/Number/White",
								digitCount, score,
								L.totalDigitW, L.totalDigitH,
								Vector3(baseX, L.totalDigitY, 0.0f),
								Vector3::One, Quaternion::Identity
							);
							m_totalDigit->m_color = COLOR_DIGIT_TOTAL;
							m_totalDigit->m_isDraw = true;
						}
					}

					if (m_drumRollHandle != app::INVALID_SE_HANDLE)
					{
						SoundManager::Get().StopSE(m_drumRollHandle);
						m_drumRollHandle = app::INVALID_SE_HANDLE; // 停止後は無効値に戻しておく
					}

					SoundManager::Get().PlaySE(enSoundKind_Cymbals);
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
						titleBackText->m_isDraw = true;
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

				const auto& L = m_dynLayout;
				float iconOffset = L.achieveOffsetY * static_cast<float>(rowIndex);
				float commonY = L.achieveStartPos.y + iconOffset;

				Vector3 currentIconPos = L.achieveStartPos;
				currentIconPos.x += L.achieveOffsetXCheck;
				currentIconPos.y = commonY;

				Vector3 currentNamePos = L.achieveStartPos;
				currentNamePos.x += L.achieveOffsetXName;
				currentNamePos.y = commonY;

				Vector3 currentBackPos = L.achieveStartPos;
				currentBackPos.x += L.achieveOffsetXBack;
				currentBackPos.y = commonY;

				std::string achieveBackKeyName = "AchieveBack_" + std::to_string(i);
				uint32_t achieveBackKey = Hash32(achieveBackKeyName.c_str());
				canvas->CreateUI<UIIcon>(achieveBackKey);
				auto* achieveBack = canvas->FindUI<UIIcon>(achieveBackKey);
				if (achieveBack)
				{
					achieveBack->Initialize("Assets/spriteData/UI/Achievement/achievementBack.DDS", L.achieveBackW, L.achieveBackH, currentBackPos, Vector3::One, Quaternion::Identity, Vector4::White);
					achieveBack->m_isDraw = true;
				}

				std::string nameKeyName = "AchieveName_" + std::to_string(i);
				uint32_t nameKey = Hash32(nameKeyName.c_str());
				canvas->CreateUI<UIText>(nameKey);
				auto* nameText = canvas->FindUI<UIText>(nameKey);
				if (nameText)
				{
					nameText->SetText(achieve->GetDescription());
					nameText->SetScale(L.achieveNameFontSize);
					nameText->m_transform.m_localTransform.m_position = currentNamePos;
					nameText->m_color = Vector4::White;
					nameText->m_pivot = Vector2(L.achieveNamePivotX, 0.5f);
					nameText->m_isDraw = true;
				}

				std::string checkBoxKeyName = "AchieveCheckBox_" + std::to_string(i);
				uint32_t checkBoxKey = Hash32(checkBoxKeyName.c_str());
				canvas->CreateUI<UIIcon>(checkBoxKey);
				auto* checkBoxIcon = canvas->FindUI<UIIcon>(checkBoxKey);
				if (checkBoxIcon)
				{
					checkBoxIcon->Initialize("Assets/spriteData/UI/Achievement/checkBox.DDS", L.achieveBoxW, L.achieveBoxH, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
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
						checkIcon->Initialize("Assets/spriteData/UI/Achievement/check.DDS", L.achieveCheckW, L.achieveCheckH, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
						checkIcon->m_isDraw = false;
						m_checkIconList.push_back(checkIcon);
					}
				}
				rowIndex++;
			}
		}
	}
}