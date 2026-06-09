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
			/** ステージ選択肢のキー */
			constexpr std::array<uint32_t, static_cast<uint8_t>(EnStageChoices::Max)> CHOICES_KEYS =
			{
				Hash32("Back"),
				Hash32("Easy"),
				Hash32("Normal"),
				Hash32("Hard"),
			};

			/** ステージ選択肢のバブルのキー */
			constexpr std::array<uint32_t, static_cast<uint8_t>(EnStageChoices::Max)> BUBBLE_KEYS =
			{
				Hash32("BackBubble"),
				Hash32("EasyBubble"),
				Hash32("NormalBubble"),
				Hash32("HardBubble"),
			};

			/** アニメーションパラメーターのパス */
			constexpr const char* ANIMATION_PARAMETER_PATH = "Assets/parameter/UI/stageSelect/StageSelectAnimationParameter.json";
			/** 選択中のアニメーションのキー */
			constexpr uint32_t SELECTING_CURSOR_ANIMATION_KEY = Hash32("SelectingBlinking");
		}




		/************************************************************************************/
		StageSelectMenu::StageChoicesData::StageChoicesData()
			: m_position(Vector3::Zero)
			, m_textIcon(nullptr)
			, m_bubbleIcon(nullptr)
		{}




		/************************************************************************************/


		StageSelectMenu::StageSelectMenu()
			: m_status(nullptr)
			, m_state(EnStageSelectState::Selecting)
			, m_selectingStage(EnStageChoices::Back)
			, m_bgIcon(nullptr)
			, m_stageSelectTextIcon(nullptr)
			, m_choices()
			, m_cursorFrame(nullptr)
			, m_cursorFrameBG(nullptr)
			, m_backBubbleFrame(nullptr)
			, m_backBubbleFrameBG(nullptr)
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
				m_cursorFrame,
				m_cursorFrameBG,
				m_backBubbleFrame,
				m_backBubbleFrameBG,
			};

			for (auto& choice : m_choices)
			{
				icons.push_back(choice.m_textIcon);
				icons.push_back(choice.m_bubbleIcon);
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
			m_selectingStage = EnStageChoices::Back;

			m_cursorFrameBG->StopAnimation();
			m_backBubbleFrameBG->StopAnimation();

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
			CheckAnimation(m_backBubbleFrameBG);



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
			constexpr float INPUT_INTERVAL = 0.2f;
			// スティックの閾値
			constexpr float STICK_INPUT_THRESHOLD = 0.5f;
			// スティックの入力値
			const float stickLXF = g_pad[0]->GetLStickXF();

			// 左にスティックを倒した場合
			if (stickLXF < -STICK_INPUT_THRESHOLD)
			{
				m_selectInputInterval = INPUT_INTERVAL;
				switch (m_selectingStage)
				{
				case EnStageChoices::Easy: m_selectingStage = EnStageChoices::Back; break;
				case EnStageChoices::Normal: m_selectingStage = EnStageChoices::Easy; break;
				case EnStageChoices::Hard:m_selectingStage = EnStageChoices::Normal; break;
				default: break;
				}
			}

			// 右にスティックを倒した場合
			else if (stickLXF > STICK_INPUT_THRESHOLD)
			{
				m_selectInputInterval = INPUT_INTERVAL;
				switch (m_selectingStage)
				{
				case EnStageChoices::Back: m_selectingStage = EnStageChoices::Easy; break;
				case EnStageChoices::Easy: m_selectingStage = EnStageChoices::Normal; break;
				case EnStageChoices::Normal: m_selectingStage = EnStageChoices::Hard; break;
				default: break;
				}
			}

			else
			{
				// インターバルをリセット
				m_selectInputInterval = 0.0f;
			}
		}


		void StageSelectMenu::UpdateSelected()
		{
			// 万が一選択されていない状態でここに来ると抜ける
			if (!m_isSelected) return;

			if (m_cursorFrameBG->IsPlayAnimation()) m_cursorFrameBG->StopAnimation();
			if (m_backBubbleFrameBG->IsPlayAnimation()) m_backBubbleFrameBG->StopAnimation();
		}


		void StageSelectMenu::UpdateDrawFlag()
		{
			m_bgIcon->SetIsDraw(true);
			m_stageSelectTextIcon->SetIsDraw(true);

			for (int i = 0; i < static_cast<int>(EnStageChoices::Max); ++i)
			{
				auto& it = m_choices.at(i);
				it.m_textIcon->SetIsDraw(true);
				it.m_bubbleIcon->SetIsDraw(true);
			}


			const bool isBackSelected = (m_selectingStage == EnStageChoices::Back);

			m_cursorFrame->SetIsDraw(!isBackSelected);
			m_cursorFrameBG->SetIsDraw(!isBackSelected);

			m_backBubbleFrame->SetIsDraw(isBackSelected);
			m_backBubbleFrameBG->SetIsDraw(isBackSelected);
		}


		void StageSelectMenu::UpdateIcons()
		{
			for (uint8_t i = 0; i < m_choices.size(); ++i)
			{
				auto& it = m_choices.at(i);

				const auto basePosition = m_status->GetChoicesBasePosition(static_cast<EnStageChoices>(i));
				const auto color = m_status->GetTextColor();
				it.m_textIcon->m_transform.m_localTransform.m_position = basePosition;
				it.m_textIcon->m_color = color;

				it.m_bubbleIcon->m_transform.m_localTransform.m_position = basePosition;
			}

			// カーソルの位置を更新
			if (m_cursorFrame && m_cursorFrameBG)
			{
				const auto basePosition = m_status->GetChoicesBasePosition(m_selectingStage);
				m_cursorFrame->m_transform.m_localTransform.m_position = basePosition;
				m_cursorFrameBG->m_transform.m_localTransform.m_position = basePosition;
			}

			if (m_backBubbleFrame && m_backBubbleFrameBG)
			{
				const auto basePosition = m_status->GetChoicesBasePosition(EnStageChoices::Back);
				m_backBubbleFrame->m_transform.m_localTransform.m_position = basePosition;
				m_backBubbleFrameBG->m_transform.m_localTransform.m_position = basePosition;
			}
		}


		void StageSelectMenu::GetUIParts()
		{
			// すでに取得している場合は取得しない

			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("BG"));
			if (!m_stageSelectTextIcon) m_stageSelectTextIcon = GetUI<UIIcon>(Hash32("StageSelect"));

			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max); ++i)
			{
				auto& it = m_choices.at(i);
				if (!it.m_textIcon) it.m_textIcon = GetUI<UIIcon>(CHOICES_KEYS.at(i));
				if (!it.m_bubbleIcon) it.m_bubbleIcon = GetUI<UIIcon>(BUBBLE_KEYS.at(i));
			}

			if (!m_cursorFrame) m_cursorFrame = GetUI<UIIcon>(Hash32("Frame"));
			if (!m_cursorFrameBG) m_cursorFrameBG = GetUI<UIIcon>(Hash32("FrameBG"));

			if (!m_backBubbleFrame) m_backBubbleFrame = GetUI<UIIcon>(Hash32("BackFrame"));
			if (!m_backBubbleFrameBG) m_backBubbleFrameBG = GetUI<UIIcon>(Hash32("BackFrameBG"));

			UIAnimationParameter::Get().Load(ANIMATION_PARAMETER_PATH);
		}


		void StageSelectMenu::SetAnimations(const uint32_t animationKey)
		{
			auto SetAnimation = [&](UIIcon* icon)
				{
					if (icon->FindAnimation(animationKey)) return;

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
					icon->AddAnimation(animationKey, std::move(anim));
				};

			SetAnimation(m_cursorFrameBG);
			SetAnimation(m_backBubbleFrameBG);
		}
	}
}