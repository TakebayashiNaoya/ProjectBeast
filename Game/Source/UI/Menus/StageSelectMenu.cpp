/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSelectMenu.h"

#include "Source/Sound/SoundManager.h"
#include "Source/UI/Animation/UIAnimation.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			const std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> CHOICES_NAME =
			{
				"Easy",
				"Normal",
				"Hard",
				"Tutorial",
			};

			const std::array<std::string, static_cast<uint8_t>(EnStageButtonTypes::Max)> BUTTON_NAME =
			{
				"Back",
				"Decide",
				"Select",
			};

			/** 選択中のアニメーションのキー */
			constexpr uint32_t SELECTING_CURSOR_ANIMATION_KEY = Hash32("SelectingBlinking");

			constexpr const char* STAGE_SELECT_JSON_PATH = "Assets/parameter/UI/stageSelect/StageSelect.json";
			constexpr const char* MENU_PARAM_KEY = "menuParam";
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
			, m_selectingStage(EnStageChoices::Easy)
			, m_bgIcon(nullptr)
			, m_stageSelectText(nullptr)
			, m_stageSelectTextBGIcon(nullptr)
			, m_choices()
			, m_buttons()
			, m_buttonBGIcon(nullptr)
			, m_cursorFrame(nullptr)
			, m_cursorFrameBG(nullptr)
			, m_stagePreviewVideo(nullptr)
			, m_prevSelectingStage(EnStageChoices::Max)
			, m_selectInputInterval(0.0f)
			, m_isSelected(false)
			, m_verticalInputDetector()
			, m_horizontalInputDetector()
			, m_cursorSelector(static_cast<int>(EnStageChoices::Max))
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
			m_stagePreviewVideo = nullptr;
			m_prevSelectingStage = EnStageChoices::Max;

			// Reload後に古い状態が残らないようリセットする。
			m_verticalInputDetector.Reset();
			m_horizontalInputDetector.Reset();
			m_cursorSelector.Reset();

			// このメニューは「倒しっぱなし連続移動」の仕様のため入力判定自体には使わないが、
			// Reload後に古い状態が残らないようリセットだけしておく。
			m_verticalInputDetector.Reset();
			m_horizontalInputDetector.Reset();
			m_cursorSelector.Reset();

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

			// JSONパラメーターを読み込む
			LoadMenuParam();

			// パーツを取得
			GetUIParts();

			// 全ステージのクリップをここで先読みしておく（カーソル移動時の I/O を排除）
			if (m_stagePreviewVideo)
			{
				m_stagePreviewVideo->ClearPreloadedClips();
				for (const auto& path : m_param.stageVideoPaths)
				{
					if (!path.empty()) m_stagePreviewVideo->PreloadClip(path.c_str());
				}
			}

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
			m_selectingStage = EnStageChoices::Easy;
			m_prevSelectingStage = EnStageChoices::Max;

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


			const float stickLXF = g_pad[0]->GetLStickXF();
			const float stickLYF = g_pad[0]->GetLStickYF();

			// 横方向：Negative=左、Positive=右。倒しっぱなし中はinputIntervalごとにリピートする。
			const auto hDir = m_horizontalInputDetector.Update(
				stickLXF, g_pad[0]->IsTrigger(enButtonLeft), g_pad[0]->IsTrigger(enButtonRight),
				m_param.inputThreshold, m_param.inputInterval);
			const bool leftInput = hDir == Direction::Negative;
			const bool rightInput = hDir == Direction::Positive;

			// 縦方向：Negative=下、Positive=上。倒しっぱなし中はinputIntervalごとにリピートする。
			const auto vDir = m_verticalInputDetector.Update(
				stickLYF, g_pad[0]->IsTrigger(enButtonDown), g_pad[0]->IsTrigger(enButtonUp),
				m_param.inputThreshold, m_param.inputInterval);
			const bool upInput = vDir == Direction::Positive;
			const bool downInput = vDir == Direction::Negative;


			auto PlayCursorSE = [&]()
				{
					SoundManager::Get().PlaySE(static_cast<int>(enSoundKind::enSoundKind_CursorMove));
				};

			if (m_selectingStage == EnStageChoices::Tutorial)
			{
				// チュートリアルから上段への移動：左→イージー、上→ノーマル、右→ハード
				if (leftInput)
				{
					m_selectingStage = EnStageChoices::Easy;
					PlayCursorSE();
				}
				else if (upInput)
				{
					m_selectingStage = EnStageChoices::Normal;
					PlayCursorSE();
				}
				else if (rightInput)
				{
					m_selectingStage = EnStageChoices::Hard;
					PlayCursorSE();
				}
			}
			else
			{
				// 上段（イージー・ノーマル・ハード）の横移動と下段への移動
				const auto current = static_cast<uint8_t>(m_selectingStage);
				constexpr uint8_t HARD_INDEX = static_cast<uint8_t>(EnStageChoices::Hard);
				if (leftInput && current > 0)
				{
					m_selectingStage = static_cast<EnStageChoices>(current - 1);
					PlayCursorSE();
				}
				else if (rightInput && current < HARD_INDEX)
				{
					m_selectingStage = static_cast<EnStageChoices>(current + 1);
					PlayCursorSE();
				}
				else if (downInput)
				{
					m_selectingStage = EnStageChoices::Tutorial;
					PlayCursorSE();
				}
			}

			// ステージが変わったら事前ロード済みクリップにポインタを切り替える（I/O なし）
			if (m_selectingStage != m_prevSelectingStage)
			{
				m_prevSelectingStage = m_selectingStage;
				if (m_stagePreviewVideo)
				{
					m_stagePreviewVideo->SwitchToPreloadedClip(static_cast<int>(m_selectingStage));
				}
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
			if (m_bgIcon)              m_bgIcon->SetIsDraw(true);
			if (m_stageSelectText)     m_stageSelectText->SetIsDraw(true);
			if (m_stageSelectTextBGIcon) m_stageSelectTextBGIcon->SetIsDraw(true);

			for (auto& it : m_choices)
			{
				if (it.m_text)       it.m_text->SetIsDraw(true);
				if (it.m_bubbleIcon) it.m_bubbleIcon->SetIsDraw(true);
			}

			for (auto& it : m_buttons)
			{
				if (it.m_button) it.m_button->SetIsDraw(true);
				if (it.m_text)   it.m_text->SetIsDraw(true);
			}

			if (m_buttonBGIcon)  m_buttonBGIcon->SetIsDraw(true);
			if (m_cursorFrame)   m_cursorFrame->SetIsDraw(true);
			if (m_cursorFrameBG) m_cursorFrameBG->SetIsDraw(true);
		}


		void StageSelectMenu::UpdateIcons()
		{
			// カーソルの位置を選択中のバブルに合わせる
			const auto& selected = m_choices.at(static_cast<uint8_t>(m_selectingStage));
			const Vector3 position = selected.m_bubbleIcon->m_transform.m_localTransform.m_position;
			m_cursorFrame->m_transform.m_localTransform.m_position = position;
			m_cursorFrameBG->m_transform.m_localTransform.m_position = position;

			// チュートリアルのバブルは横幅が広いのでカーソルを拡大する
			const Vector3 cursorScale = (m_selectingStage == EnStageChoices::Tutorial)
				? Vector3(m_param.tutorialCursorScaleX, 1.0f, 1.0f)
				: Vector3::One;
			m_cursorFrame->m_transform.m_localTransform.m_scale = cursorScale;
			m_cursorFrameBG->m_transform.m_localTransform.m_scale = cursorScale;
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

			if (!m_stagePreviewVideo) m_stagePreviewVideo = GetUI<UIVideo>(Hash32("StagePreviewVideo"));
		}


		void StageSelectMenu::SetAnimations(const uint32_t animationKey)
		{
			if (m_cursorFrameBG->FindAnimation(animationKey)) return;

			auto anim = std::make_unique<UIColorAnimation>();
			anim->SetParameter(
				m_param.cursorBlinkStartColor,
				m_param.cursorBlinkEndColor,
				m_param.cursorBlinkDuration,
				util::EasingType::EaseInOut,
				util::LoopMode::PingPong
			);
			m_cursorFrameBG->AddAnimation(animationKey, std::move(anim));
		}


		void StageSelectMenu::LoadMenuParam()
		{
			nlohmann::json json;
			if (!app::util::JsonConverter::IsLoadJsonFile(json, STAGE_SELECT_JSON_PATH)) return;
			if (!json.contains(MENU_PARAM_KEY)) return;

			const auto& p = json[MENU_PARAM_KEY];
			using JC = app::util::JsonConverter;

			m_param.inputInterval = JC::ToFloat(p, "inputInterval", m_param.inputInterval);
			m_param.inputThreshold = JC::ToFloat(p, "inputThreshold", m_param.inputThreshold);
			m_param.tutorialCursorScaleX = JC::ToFloat(p, "tutorialCursorScaleX", m_param.tutorialCursorScaleX);
			m_param.cursorBlinkDuration = JC::ToFloat(p, "cursorBlinkDuration", m_param.cursorBlinkDuration);
			m_param.cursorBlinkStartColor = JC::ToVector4(p, "cursorBlinkStartColor", true, m_param.cursorBlinkStartColor);
			m_param.cursorBlinkEndColor = JC::ToVector4(p, "cursorBlinkEndColor", true, m_param.cursorBlinkEndColor);

			if (p.contains("stageVideoPaths") && p["stageVideoPaths"].is_array())
			{
				const auto& paths = p["stageVideoPaths"];
				for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max) && i < paths.size(); ++i)
				{
					m_param.stageVideoPaths[i] = paths[i].get<std::string>();
				}
			}
		}

	}
}
