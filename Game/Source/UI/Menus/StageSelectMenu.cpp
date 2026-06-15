/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSelectMenu.h"

#include "Source/UI/Animation/UIAnimation.h"


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

			/** 選択中のアニメーションのキー */
			constexpr uint32_t SELECTING_CURSOR_ANIMATION_KEY = Hash32("SelectingBlinking");

			constexpr float INPUT_INTERVAL  = 0.2f;
			constexpr float INPUT_THRESHOLD = 0.5f;

			/** カーソル点滅アニメーションのパラメーター */
			const Vector4 CURSOR_BLINK_START    = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			const Vector4 CURSOR_BLINK_END      = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			constexpr float CURSOR_BLINK_DURATION = 0.5f;
		}




		/************************************************************************************/
		StageSelectMenu::StageChoicesData::StageChoicesData()
			: m_text(nullptr)
			, m_bubbleIcon(nullptr)
		{}




		/************************************************************************************/

		StageSelectMenu::StageButtonData::StageButtonData()
			: m_button(nullptr)
			, m_text(nullptr)
		{}




		/************************************************************************************/


		StageSelectMenu::StageSelectMenu()
			: m_state(EnStageSelectState::Selecting)
			, m_selectingStage(EnStageChoices::Tutorial)
			, m_bgIcon(nullptr)
			, m_stageSelectText(nullptr)
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
			// Reload後に古いポインタが残らないようリセット
			m_bgIcon = nullptr;
			m_stageSelectText = nullptr;
			m_stageSelectTextBGIcon = nullptr;
			m_buttonBGIcon = nullptr;
			m_cursorFrame = nullptr;
			m_cursorFrameBG = nullptr;
			for (auto& choice : m_choices)
			{
				choice.m_text = nullptr;
				choice.m_bubbleIcon = nullptr;
			}
			for (auto& button : m_buttons)
			{
				button.m_button = nullptr;
				button.m_text = nullptr;
			}

			// パーツを取得
			GetUIParts();

			std::vector<UIBase*> icons = {
				m_bgIcon,
				m_stageSelectText,
				m_stageSelectTextBGIcon,
				m_cursorFrame,
				m_cursorFrameBG,
				m_buttonBGIcon,
			};

			for (auto& choice : m_choices)
			{
				icons.push_back(choice.m_text);
				icons.push_back(choice.m_bubbleIcon);
			}

			for (auto& button : m_buttons)
			{
				icons.push_back(button.m_button);
				icons.push_back(button.m_text);
			}

			for (const auto& icon : icons)
			{
				K2_ASSERT(icon, "アイコンを取得できていません。");

				icon->m_isDraw = false;
			}

		}


		void StageSelectMenu::Update()
		{
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

			const float stickLXF = g_pad[0]->GetLStickXF();

			// 左にスティックを倒した場合
			const auto current = static_cast<uint8_t>(m_selectingStage);
			const auto max = static_cast<uint8_t>(EnStageChoices::Max);
			if (stickLXF < -INPUT_THRESHOLD && current > 0)
			{
				m_selectInputInterval = INPUT_INTERVAL;
				m_selectingStage = static_cast<EnStageChoices>(current - 1);
			}
			else if (stickLXF > INPUT_THRESHOLD && current < max - 1)
			{
				m_selectInputInterval = INPUT_INTERVAL;
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
			m_stageSelectText->SetIsDraw(true);
			m_stageSelectTextBGIcon->SetIsDraw(true);

			for (auto& it : m_choices)
			{
				it.m_text->SetIsDraw(true);
				it.m_bubbleIcon->SetIsDraw(true);
			}

			for (auto& it : m_buttons)
			{
				it.m_button->SetIsDraw(true);
				it.m_text->SetIsDraw(true);
			}

			m_buttonBGIcon->SetIsDraw(true);

			m_cursorFrame->SetIsDraw(true);
			m_cursorFrameBG->SetIsDraw(true);
		}


		void StageSelectMenu::UpdateIcons()
		{
			// カーソルの位置を選択中のバブルに合わせる
			const auto& selected = m_choices.at(static_cast<uint8_t>(m_selectingStage));
			const Vector3 position = selected.m_bubbleIcon->m_transform.m_localTransform.m_position;
			m_cursorFrame->m_transform.m_localTransform.m_position = position;
			m_cursorFrameBG->m_transform.m_localTransform.m_position = position;
		}


		void StageSelectMenu::GetUIParts()
		{
			// すでに取得している場合は取得しない

			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("BG"));
			if (!m_stageSelectText) m_stageSelectText = GetUI<UIText>(Hash32("StageSelectText"));
			if (!m_stageSelectTextBGIcon) m_stageSelectTextBGIcon = GetUI<UIIcon>(Hash32("StageSelectBG"));


			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max); ++i)
			{
				auto& it = m_choices.at(i);
				const auto textKey = CHOICES_NAME.at(i) + "Text";
				const auto bubbleKey = CHOICES_NAME.at(i) + "Bubble";
				if (!it.m_text) it.m_text = GetUI<UIText>(Hash32(textKey.c_str()));
				if (!it.m_bubbleIcon) it.m_bubbleIcon = GetUI<UIIcon>(Hash32(bubbleKey.c_str()));
			}


			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageButtonTypes::Max); ++i)
			{
				auto& it = m_buttons.at(i);
				const auto buttonKey = BUTTON_NAME.at(i) + "Button";
				const auto textKey = BUTTON_NAME.at(i) + "Text";
				if (!it.m_button) it.m_button = GetUI<UIIcon>(Hash32(buttonKey.c_str()));
				if (!it.m_text) it.m_text = GetUI<UIText>(Hash32(textKey.c_str()));
			}

			if (!m_buttonBGIcon) m_buttonBGIcon = GetUI<UIIcon>(Hash32("ButtonBG"));

			if (!m_cursorFrame) m_cursorFrame = GetUI<UIIcon>(Hash32("Frame"));
			if (!m_cursorFrameBG) m_cursorFrameBG = GetUI<UIIcon>(Hash32("FrameBG"));
		}


		void StageSelectMenu::SetAnimations(const uint32_t animationKey)
		{
			if (m_cursorFrameBG->FindAnimation(animationKey)) return;

			auto anim = std::make_unique<UIColorAnimation>();
			anim->SetParameter(
				CURSOR_BLINK_START,
				CURSOR_BLINK_END,
				CURSOR_BLINK_DURATION,
				util::EasingType::EaseInOut,
				util::LoopMode::PingPong
			);
			m_cursorFrameBG->AddAnimation(animationKey, std::move(anim));
		}

	}
}
