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
	// アニメーション用タイマー
	constexpr float CHECK_REVEAL_DELAY = 1.0f;
	constexpr float CHECK_REVEAL_INTERVAL = 1.0f;
	constexpr float TOTAL_REVEAL_DELAY = 1.0f;
	constexpr float COUNT_UP_DURATION = 4.5f;  // トータルスコアのカウントアップにかける時間
	constexpr float TITLE_BUTTON_DELAY = 1.5f;

	constexpr float ACHIEVE_X_POS = 100.0f;

	constexpr const char* RESULT_JSON_PATH = "Assets/parameter/result/result.json";
	constexpr const char* DYNAMIC_LAYOUT_KEY = "dynamic_layout";
}

namespace app
{
	namespace ui
	{
		ResultMenu::ResultMenu()
			: m_collectedPenguin(0)
			, m_totalScore(0.0f)
			, m_checkRevealTimer(0.0f)
			, m_checkRevealIndex(0)
			, m_allChecksRevealed(false)
			, m_postCheckTimer(0.0f)
			, m_countUpTimer(0.0f)
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
			m_countUpTimer = 0.0f;
			m_totalScoreShown = false;
			m_titleButtonShown = false;
			m_checkIconList.clear();

			ClearDynamicElements();
			BuildDynamicUI();
		}


		void ResultMenu::SetResultData(int collectedPenguin, float totalScore, const std::vector<app::achievement::AchievementBase*>& achievements, float scorePerAchieve)
		{
			m_collectedPenguin = collectedPenguin;
			m_totalScore = totalScore;
			m_allAchievementList = achievements;
			m_scorePerAchieve = scorePerAchieve;
			m_dataSet = true;

			ClearDynamicElements();
			BuildDynamicUI();
		}


		void ResultMenu::ClearDynamicElements()
		{
			m_scorePopups.clear();

			auto* canvas = GetCanvas();
			if (!canvas) return;

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

		}


		void ResultMenu::BuildDynamicUI()
		{
			auto* canvas = GetCanvas();
			if (!canvas) return;

			LoadDynamicLayout();

			// タイトルへ戻るUI一式は非表示にしておく
			auto* titleBackText = GetUI<UIText>(Hash32("TitleBackText"));
			if (titleBackText) titleBackText->m_isDraw = false;

			auto* titleBackFrame = GetUI<UIIcon>(Hash32("TitleBackFrame"));
			if (titleBackFrame) titleBackFrame->m_isDraw = false;

			auto* titleBackAButton = GetUI<UIIcon>(Hash32("TitleBackAButton"));
			if (titleBackAButton) titleBackAButton->m_isDraw = false;

			// トータルは演出で表示するため初期非表示
			auto* totalValue = GetUI<UIText>(Hash32("TotalValue"));
			if (totalValue) totalValue->m_isDraw = false;

			// ---------------------------------------------------------
			// 助けた数の初期テキスト設定
			// ---------------------------------------------------------
			auto* rescueValue = GetUI<UIText>(Hash32("RescueValue"));
			if (rescueValue) rescueValue->SetText(std::to_string(m_collectedPenguin));

			SetupAchievementUI();
		}


		void ResultMenu::Update()
		{
			for (auto& popup : m_scorePopups)
			{
				popup->Update();
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
						SoundManager::Get().PlaySE(enSoundKind_Stamp, 1.0f);

						// アチーブの横に「+2000」をポップアップさせる
						if (m_checkRevealIndex < static_cast<int>(m_scorePopups.size()))
						{
							m_scorePopups[m_checkRevealIndex]->Play(static_cast<int>(m_scorePerAchieve));
						}

						m_checkRevealIndex++;
					}
				}
				else
				{
					m_postCheckTimer += dt;
					if (m_postCheckTimer >= TOTAL_REVEAL_DELAY)
					{
						m_allChecksRevealed = true;
						m_postCheckTimer = 0.0f;

						// ここからトータルスコアのカウントアップ開始のためドラムロールを鳴らす
						m_drumRollHandle = SoundManager::Get().PlaySE(enSoundKind_DrumRoll, 1.0f, false);

						auto* totalValue = GetUI<UIText>(Hash32("TotalValue"));
						if (totalValue)
						{
							totalValue->m_isDraw = true;
							totalValue->SetText("0");
						}
					}
				}
				return;
			}

			// フェーズ2：トータルスコアをカウントアップ
			if (!m_totalScoreShown)
			{
				m_countUpTimer += dt;
				float progress = m_countUpTimer / COUNT_UP_DURATION;
				if (progress > 1.0f) progress = 1.0f;

				// 0から目標の合計スコアまで徐々に増やす
				float currentScore = m_totalScore * progress;

				auto* totalValue = GetUI<UIText>(Hash32("TotalValue"));
				if (totalValue)
				{
					totalValue->SetText(std::to_string(static_cast<int>(currentScore)));
				}

				if (progress >= 1.0f)
				{
					if (m_drumRollHandle != app::INVALID_SE_HANDLE)
					{
						SoundManager::Get().StopSE(m_drumRollHandle);
						m_drumRollHandle = app::INVALID_SE_HANDLE;
					}

					SoundManager::Get().PlaySE(enSoundKind_Cymbals);
					m_totalScoreShown = true;
					m_postCheckTimer = 0.0f;
				}
				return;
			}

			// フェーズ3：Aボタンガイドを表示
			if (!m_titleButtonShown)
			{
				m_postCheckTimer += dt;
				if (m_postCheckTimer >= TITLE_BUTTON_DELAY)
				{
					auto* titleBackText = GetUI<UIText>(Hash32("TitleBackText"));
					if (titleBackText) titleBackText->m_isDraw = true;

					auto* titleBackFrame = GetUI<UIIcon>(Hash32("TitleBackFrame"));
					if (titleBackFrame) titleBackFrame->m_isDraw = true;

					auto* titleBackAButton = GetUI<UIIcon>(Hash32("TitleBackAButton"));
					if (titleBackAButton) titleBackAButton->m_isDraw = true;

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

					Vector3 popupPos = currentBackPos;
					popupPos.x += L.achieveBackW * 0.5f - ACHIEVE_X_POS;  // 枠右端付近（調整可）

					auto popup = std::make_unique<ScorePopupAnimator>();
					popup->Initialize(popupPos);
					m_scorePopups.push_back(std::move(popup));
				}
				rowIndex++;
			}
		}


		void ResultMenu::Render(RenderContext& rc)
		{
			MenuBase::Render(rc);   // 通常のUIを先に描画

			// ポップアップをUIの上に重ねて描画
			for (auto& popup : m_scorePopups)
			{
				popup->Render(rc);
			}
		}
	}
}