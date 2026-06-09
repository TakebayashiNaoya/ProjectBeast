/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSelectMenu.h"

#include "StageSelectStatus.h"

#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			const std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> CHOICES_NAME =
			{
				"Tutorial",
				"Easy",
				"Normal",
				"Hard",
			};

			const std::array<std::string, static_cast<uint8_t>(EnStageButtonTypes::Max)> BUTTON_NAME =
			{
				"Back",
				"Decide",
				"Select",
			};

			/** アニメーションパラメーターのパス */
			constexpr const char* ANIMATION_PARAMETER_PATH = "Assets/parameter/UI/stageSelect/StageSelectAnimationParameter.json";
			/** 選択中のアニメーションのキー */
			constexpr uint32_t SELECTING_CURSOR_ANIMATION_KEY = Hash32("SelectingBlinking");
		}




		/************************************************************************************/
		StageSelectMenu::StageChoicesData::StageChoicesData()
			: m_textIcon(nullptr)
			, m_bubbleIcon(nullptr)
		{}




		/************************************************************************************/

		StageSelectMenu::StageButtonData::StageButtonData()
			: m_button(nullptr)
			, m_textIcon(nullptr)
		{}




		/************************************************************************************/


		StageSelectMenu::StageSelectMenu()
			: m_status(nullptr)
			, m_state(EnStageSelectState::Selecting)
			, m_selectingStage(EnStageChoices::Tutorial)
			, m_bgIcon(nullptr)
			, m_stageSelectTextIcon(nullptr)
			, m_stageSelectTextBGIcon(nullptr)
			, m_choices()
			, m_buttons()
			, m_buttonBGIcon(nullptr)
			, m_cursorFrame(nullptr)
			, m_cursorFrameBG(nullptr)
			, m_selectInputInterval(0.0f)
			, m_isSelected(false)
		{}


		StageSelectMenu::~StageSelectMenu()
		{}


		void StageSelectMenu::InitializeLogic()
		{
			m_status.reset();
			m_status = std::make_unique<StageSelectStatus>();
			m_status->SetUp();

			K2_ASSERT(m_status.get(), "ステータス生成失敗です。");

			// パーツを取得
			GetUIParts();

			std::vector<UIIcon*> icons = {
				m_bgIcon,
				m_stageSelectTextIcon,
				m_stageSelectTextBGIcon,
				m_cursorFrame,
				m_cursorFrameBG,
				m_buttonBGIcon,
			};

			for (auto& choice : m_choices)
			{
				icons.push_back(choice.m_textIcon);
				icons.push_back(choice.m_bubbleIcon);
			}

			for (auto& button : m_buttons)
			{
				icons.push_back(button.m_button);
				icons.push_back(button.m_textIcon);
			}

			for (const auto& icon : icons)
			{
				K2_ASSERT(icon, "アイコンを取得できていません。");

				icon->m_isDraw = false;
			}

			UIAnimationParameter::Get().Load(ANIMATION_PARAMETER_PATH);
		}


		void StageSelectMenu::Update()
		{
			if (!m_status->IsSetUp())
			{
				m_status->SetUp();
				return;
			}

			// ステージ選択状態によって処理を分ける
			switch (m_state)
			{
			case EnStageSelectState::Selecting:
			{
				UpdateSelecting();
				break;
			}
			case EnStageSelectState::Selected:
			{
				UpdateSelected();
				break;
			}
			}


			// 描画フラグを更新
			UpdateDrawFlag();
			UpdateIcons();

			// Canvasの更新
			MenuBase::Update();
		}


		void StageSelectMenu::Reset()
		{
			m_state = EnStageSelectState::Selecting;
			m_selectingStage = EnStageChoices::Tutorial;

			m_cursorFrameBG->StopAnimation();

			m_isSelected = false;
		}


		void StageSelectMenu::UpdateSelecting()
		{
			// 選択済みになると状態を変更して抜ける
			if (m_isSelected)
			{
				m_state = EnStageSelectState::Selected;
				return;
			}


			auto CheckAnimation = [&](UIIcon* icon)
				{
					if (icon && !icon->IsPlayAnimation())
					{
						SetAnimations(SELECTING_CURSOR_ANIMATION_KEY);
						icon->PlayAnimation();
					}
				};

			CheckAnimation(m_cursorFrameBG);



			if (m_selectInputInterval > 0.0f)
			{
				m_selectInputInterval -= g_gameTime->GetFrameDeltaTime();

				// タイマーがまだ残っていたら、これ以上の入力処理はせずに抜ける
				if (m_selectInputInterval > 0.0f)
				{
					return;
				}
			}

			// 入力のインターバル
			const float inputInterval = m_status->GetInputInterval();
			// スティックの閾値
			const float inputThreshold = m_status->GetInputThreshold();
			// スティックの入力値
			const float stickLXF = g_pad[0]->GetLStickXF();

			// 左にスティックを倒した場合
			const auto current = static_cast<uint8_t>(m_selectingStage);
			const auto max = static_cast<uint8_t>(EnStageChoices::Max);
			if (stickLXF < -inputThreshold && current > 0)
			{
				m_selectInputInterval = inputInterval;
				m_selectingStage = static_cast<EnStageChoices>(current - 1);
			}
			else if (stickLXF > inputThreshold && current < max - 1)
			{
				m_selectInputInterval = inputInterval;
				m_selectingStage = static_cast<EnStageChoices>(current + 1);
			}
		}


		void StageSelectMenu::UpdateSelected()
		{
			// 万が一選択されていない状態でここに来ると抜ける
			if (!m_isSelected) return;

			if (m_cursorFrameBG->IsPlayAnimation()) m_cursorFrameBG->StopAnimation();
		}


		void StageSelectMenu::UpdateDrawFlag()
		{
			m_bgIcon->SetIsDraw(true);
			m_stageSelectTextIcon->SetIsDraw(true);
			m_stageSelectTextBGIcon->SetIsDraw(true);

			for (auto& it : m_choices)
			{
				it.m_textIcon->SetIsDraw(true);
				it.m_bubbleIcon->SetIsDraw(true);
			}

			for (auto& it : m_buttons)
			{
				it.m_button->SetIsDraw(true);
				it.m_textIcon->SetIsDraw(true);
			}

			m_buttonBGIcon->SetIsDraw(true);

			m_cursorFrame->SetIsDraw(true);
			m_cursorFrameBG->SetIsDraw(true);
		}


		void StageSelectMenu::UpdateIcons()
		{
			const auto textBGColor = m_status->GetTextBGColor();

			const auto textPosition = m_status->GetStageSelectPosition();
			m_stageSelectTextIcon->m_transform.m_localTransform.m_position = textPosition;
			m_stageSelectTextBGIcon->m_transform.m_localTransform.m_position = textPosition;
			m_stageSelectTextBGIcon->m_color = textBGColor;


			m_buttonBGIcon->m_transform.m_localTransform.m_position = m_status->GetButtonBGPosition();
			m_buttonBGIcon->m_color = textBGColor;


			// 選択肢の位置と色を更新
			for (uint8_t i = 0; i < m_choices.size(); ++i)
			{
				auto& it = m_choices.at(i);

				const auto x = m_status->GetChoicePositionX(static_cast<EnStageChoices>(i));
				const auto y = m_status->GetChoicesYOffset();
				const Vector3 position = Vector3(x, y, 0.0f);

				const auto color = m_status->GetChoicesTextColor();
				it.m_textIcon->m_transform.m_localTransform.m_position = position;
				it.m_textIcon->m_color = color;

				it.m_bubbleIcon->m_transform.m_localTransform.m_position = position;
			}


			// ボタンの位置を更新
			for (uint8_t i = 0; i < m_buttons.size(); ++i)
			{
				auto& it = m_buttons.at(i);
				const auto x = m_status->GetButtonPositionX(static_cast<EnStageButtonTypes>(i));
				const auto y = m_status->GetButtonYOffset();
				const Vector3 iconPosition = Vector3(x, y, 0.0f);
				const Vector3 textPosition = Vector3(x + m_status->GetButtonXOffset(), y, 0.0f);
				it.m_button->m_transform.m_localTransform.m_position = iconPosition;
				it.m_textIcon->m_transform.m_localTransform.m_position = textPosition;
			}


			// カーソルの位置を更新

			const auto x = m_status->GetChoicePositionX(m_selectingStage);
			const auto y = m_status->GetChoicesYOffset();
			const Vector3 position = Vector3(x, y, 0.0f);
			m_cursorFrame->m_transform.m_localTransform.m_position = position;
			m_cursorFrameBG->m_transform.m_localTransform.m_position = position;
		}


		void StageSelectMenu::GetUIParts()
		{
			// すでに取得している場合は取得しない

			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("BG"));
			if (!m_stageSelectTextIcon) m_stageSelectTextIcon = GetUI<UIIcon>(Hash32("StageSelectText"));
			if (!m_stageSelectTextBGIcon) m_stageSelectTextBGIcon = GetUI<UIIcon>(Hash32("StageSelectBG"));


			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max); ++i)
			{
				auto& it = m_choices.at(i);
				const auto textKey = CHOICES_NAME.at(i) + "Text";
				const auto bubbleKey = CHOICES_NAME.at(i) + "Bubble";
				if (!it.m_textIcon) it.m_textIcon = GetUI<UIIcon>(Hash32(textKey.c_str()));
				if (!it.m_bubbleIcon) it.m_bubbleIcon = GetUI<UIIcon>(Hash32(bubbleKey.c_str()));
			}


			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageButtonTypes::Max); ++i)
			{
				auto& it = m_buttons.at(i);
				const auto buttonKey = BUTTON_NAME.at(i) + "Button";
				const auto textKey = BUTTON_NAME.at(i) + "Text";
				if (!it.m_button) it.m_button = GetUI<UIIcon>(Hash32(buttonKey.c_str()));
				if (!it.m_textIcon) it.m_textIcon = GetUI<UIIcon>(Hash32(textKey.c_str()));
			}

			if (!m_buttonBGIcon) m_buttonBGIcon = GetUI<UIIcon>(Hash32("ButtonBG"));

			if (!m_cursorFrame) m_cursorFrame = GetUI<UIIcon>(Hash32("Frame"));
			if (!m_cursorFrameBG) m_cursorFrameBG = GetUI<UIIcon>(Hash32("FrameBG"));
		}


		void StageSelectMenu::SetAnimations(const uint32_t animationKey)
		{
			if (m_cursorFrameBG->FindAnimation(animationKey)) return;

			auto param = UIAnimationParameter::Get().Find(animationKey);
			if (!param) return;
			auto anim = std::make_unique<UIColorAnimation>();

			anim->SetParameter(
				param->startV4,
				param->endV4,
				param->duration,
				param->easingType,
				param->loopMode
			);
			m_cursorFrameBG->AddAnimation(animationKey, std::move(anim));
		}

	}
}