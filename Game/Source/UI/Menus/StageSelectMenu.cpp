/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSelectMenu.h"

#include "Source/Manager/ScoreManager.h"
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

			/** カーソル移動ポップの長さ（秒）と大きさ（タイトル画面と同じ手応えに揃える） */
			constexpr float CURSOR_POP_DURATION = 0.15f;
			constexpr float CURSOR_POP_SCALE = 0.2f;

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
			, m_selectFlashIcon(nullptr)
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
			m_selectFlashIcon = nullptr;
			m_prevSelectingStage = EnStageChoices::Max;

			// 選択確定演出の状態もリセットする
			m_zoomTargets.clear();
			m_isZoomBaseCaptured = false;
			m_selectEffectTimer = 0.0f;

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
				m_selectFlashIcon,
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
			UpdateStageInfo();

			// 選択確定演出（位置・スケールを上書きするので各Updateの後に行う）
			UpdateSelectEffect();

			// Canvasの更新
			MenuBase::Update();
		}


		void StageSelectMenu::LoadStageInfoIfNeeded()
		{
			if (m_isStageInfoLoaded) return;
			m_isStageInfoLoaded = true;

			/** 配置JSONの実データから数を数える。
			 *  ステージを再生成しても表示が自動で追従する */
			constexpr const char* ENEMY_PATHS[STAGE_INFO_NUM] = {
				"Assets/parameter/character/enemy/EnemyLayout_Easy.json",
				"Assets/parameter/character/enemy/EnemyLayout_Normal.json",
				"Assets/parameter/character/enemy/EnemyLayout_Hard.json",
			};
			constexpr const char* WHIRL_PATHS[STAGE_INFO_NUM] = {
				"Assets/parameter/stage/whirlpoolPositions_Easy.json",
				"Assets/parameter/stage/whirlpoolPositions_Normal.json",
				"Assets/parameter/stage/whirlpoolPositions_Hard.json",
			};

			for (int i = 0; i < STAGE_INFO_NUM; ++i)
			{
				nlohmann::json json;
				if (util::JsonConverter::IsLoadJsonFile(json, ENEMY_PATHS[i])
					&& json.contains("enemies"))
				{
					m_stageBearCounts[i] = static_cast<int>(json["enemies"].size());
				}
				if (util::JsonConverter::IsLoadJsonFile(json, WHIRL_PATHS[i])
					&& json.contains("whirlpoolPositions"))
				{
					m_stageWhirlCounts[i] = static_cast<int>(json["whirlpoolPositions"].size());
				}
			}
		}


		void StageSelectMenu::UpdateStageInfo()
		{
			auto* panel = GetUI<UIIcon>(Hash32("StageInfoPanel"));
			auto* timeIcon = GetUI<UIIcon>(Hash32("StageInfoTimeIcon"));
			auto* timeText = GetUI<UIText>(Hash32("StageInfoTimeText"));
			auto* bearIcon = GetUI<UIIcon>(Hash32("StageInfoBearIcon"));
			auto* bearText = GetUI<UIText>(Hash32("StageInfoBearText"));
			auto* whirlIcon = GetUI<UIIcon>(Hash32("StageInfoWhirlIcon"));
			auto* whirlText = GetUI<UIText>(Hash32("StageInfoWhirlText"));
			auto* recordText = GetUI<UIText>(Hash32("StageInfoRecordText"));
			if (!panel || !timeIcon || !timeText || !bearIcon || !bearText
				|| !whirlIcon || !whirlText || !recordText)
			{
				return;
			}

			/** チュートリアル選択中と選択確定後は出さない */
			const bool isShow = (m_state == EnStageSelectState::Selecting)
				&& (m_selectingStage != EnStageChoices::Tutorial);
			panel->m_isDraw = isShow;
			timeIcon->m_isDraw = isShow;
			timeText->m_isDraw = isShow;
			bearIcon->m_isDraw = isShow;
			bearText->m_isDraw = isShow;
			whirlIcon->m_isDraw = isShow;
			whirlText->m_isDraw = isShow;
			recordText->m_isDraw = isShow;
			if (!isShow) return;

			LoadStageInfoIfNeeded();

			/** 制限時間は各シーンヘッダの GetTimeLimit() と同期させること */
			constexpr int STAGE_TIME_SECONDS[STAGE_INFO_NUM] = { 120, 150, 180 };
			constexpr const char* STAGE_NAMES[STAGE_INFO_NUM] = { "Easy", "Normal", "Hard" };

			const int index = static_cast<int>(m_selectingStage);
			if (index < 0 || index >= STAGE_INFO_NUM) return;

			char buf[48];
			sprintf_s(buf, "%d:%02d", STAGE_TIME_SECONDS[index] / 60, STAGE_TIME_SECONDS[index] % 60);
			timeText->SetText(buf);

			sprintf_s(buf, "x%d", m_stageBearCounts[index]);
			bearText->SetText(buf);

			sprintf_s(buf, "x%d", m_stageWhirlCounts[index]);
			whirlText->SetText(buf);

			const int highScore = ScoreManager::GetHighScore(STAGE_NAMES[index]);
			if (highScore > 0)
			{
				sprintf_s(buf, "きろく %d", highScore);
				recordText->SetText(buf);
			}
			else
			{
				recordText->SetText("きろく ---");
			}
		}


		void StageSelectMenu::CaptureZoomBase()
		{
			m_zoomTargets.clear();

			// この時点で描画されている全パーツを対象にする（白フラッシュは全画面のまま残すので除外）
			std::vector<UIBase*> targets = {
				m_bgIcon,
				m_stagePreviewVideo,
				m_stageSelectText,
				m_stageSelectTextBGIcon,
				m_buttonBGIcon,
				m_cursorFrame,
				m_cursorFrameBG,
			};
			for (auto& choice : m_choices)
			{
				targets.push_back(choice.m_text);
				targets.push_back(choice.m_bubbleIcon);
			}
			for (auto& button : m_buttons)
			{
				targets.push_back(button.m_button);
				targets.push_back(button.m_text);
			}

			// 現在の位置・スケールを基準値として保存する
			for (auto* ui : targets)
			{
				if (ui == nullptr) continue;

				ZoomTarget target;
				target.m_ui = ui;
				target.m_basePosition = ui->m_transform.m_localTransform.m_position;
				target.m_baseScale = ui->m_transform.m_localTransform.m_scale;
				if (auto* text = dynamic_cast<UIText*>(ui))
				{
					target.m_baseFontScale = text->GetScale();
				}
				m_zoomTargets.push_back(target);
			}

			m_isZoomBaseCaptured = true;
		}


		void StageSelectMenu::UpdateSelectEffect()
		{
			if (m_state != EnStageSelectState::Selected) return;

			// 演出の開始フレームで基準値を保存する
			if (!m_isZoomBaseCaptured)
			{
				CaptureZoomBase();
			}

			m_selectEffectTimer += g_gameTime->GetFrameDeltaTime();

			// 画面中央へ加速しながら吸い込まれるズーム（座標系が中央原点なので位置×倍率で放射拡大になる）
			const float u = (std::min)(m_selectEffectTimer / m_param.selectZoomDuration, 1.0f);
			const float zoom = 1.0f + (m_param.selectZoomScale - 1.0f) * u * u;
			for (auto& target : m_zoomTargets)
			{
				auto& transform = target.m_ui->m_transform.m_localTransform;
				transform.m_position = Vector3(
					target.m_basePosition.x * zoom, target.m_basePosition.y * zoom, target.m_basePosition.z);

				// テキストのスケールはフォントスケールで、それ以外はトランスフォームで掛ける
				if (auto* text = dynamic_cast<UIText*>(target.m_ui))
				{
					text->SetScale(Vector2(target.m_baseFontScale.x * zoom, target.m_baseFontScale.y * zoom));
				}
				else
				{
					transform.m_scale = Vector3(
						target.m_baseScale.x * zoom, target.m_baseScale.y * zoom, target.m_baseScale.z);
				}
			}

			// ズームの後半で徐々に真っ白へ（このあと既存のシーンフェードで暗転しロードへつながる）
			if (m_selectFlashIcon)
			{
				const float fadeStart = (std::max)(m_param.selectZoomDuration - m_param.selectWhiteFadeDuration, 0.0f);
				const float fade = (m_param.selectWhiteFadeDuration > 0.0f)
					? (m_selectEffectTimer - fadeStart) / m_param.selectWhiteFadeDuration
					: 1.0f;
				const float alpha = (std::max)(0.0f, (std::min)(fade, 1.0f));
				m_selectFlashIcon->SetIsDraw(alpha > 0.0f);
				m_selectFlashIcon->m_color.w = alpha;
			}
		}


		void StageSelectMenu::Reset()
		{
			m_state = EnStageSelectState::Selecting;
			m_selectingStage = EnStageChoices::Easy;
			m_prevSelectingStage = EnStageChoices::Max;

			m_cursorFrameBG->StopAnimation();

			// 選択確定演出を巻き戻す（ズームした位置・スケールを元に戻す）
			if (m_isZoomBaseCaptured)
			{
				for (auto& target : m_zoomTargets)
				{
					target.m_ui->m_transform.m_localTransform.m_position = target.m_basePosition;
					if (auto* text = dynamic_cast<UIText*>(target.m_ui))
					{
						text->SetScale(target.m_baseFontScale);
					}
					else
					{
						target.m_ui->m_transform.m_localTransform.m_scale = target.m_baseScale;
					}
				}
				m_zoomTargets.clear();
				m_isZoomBaseCaptured = false;
			}
			m_selectEffectTimer = 0.0f;
			if (m_selectFlashIcon)
			{
				m_selectFlashIcon->SetIsDraw(false);
				m_selectFlashIcon->m_color.w = 0.0f;
			}

			m_cursorPopTimer = 0.0f;
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


			// カーソル移動の手応え：SEとフレームのポップ（タイトル画面と同じ演出）
			auto PlayCursorSE = [&]()
				{
					SoundManager::Get().PlaySE(static_cast<int>(enSoundKind::enSoundKind_CursorMove));
					m_cursorPopTimer = CURSOR_POP_DURATION;
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

			// カーソル移動のポップ（フレームが一瞬大きくなって戻る）
			if (m_cursorPopTimer > 0.0f)
			{
				m_cursorPopTimer -= g_gameTime->GetFrameDeltaTime();
			}
			const float pop = 1.0f
				+ CURSOR_POP_SCALE * (std::max)(m_cursorPopTimer, 0.0f) / CURSOR_POP_DURATION;

			// チュートリアルのバブルは横幅が広いのでカーソルを拡大する
			const Vector3 cursorScale = (m_selectingStage == EnStageChoices::Tutorial)
				? Vector3(m_param.tutorialCursorScaleX * pop, pop, 1.0f)
				: Vector3(pop, pop, 1.0f);
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

			if (!m_selectFlashIcon) m_selectFlashIcon = GetUI<UIIcon>(Hash32("SelectFlashWhite"));
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
			m_param.selectZoomDuration = JC::ToFloat(p, "selectZoomDuration", m_param.selectZoomDuration);
			m_param.selectZoomScale = JC::ToFloat(p, "selectZoomScale", m_param.selectZoomScale);
			m_param.selectWhiteFadeDuration = JC::ToFloat(p, "selectWhiteFadeDuration", m_param.selectWhiteFadeDuration);
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
