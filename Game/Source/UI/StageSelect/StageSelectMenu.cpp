/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
 */
#include "stdafx.h"
#include "StageSelectMenu.h"

#include "Source/Sound/SoundManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/**
			 * 選択中だけ見せるパーツのうち、表示/非表示にしか使わないもの。
			 * ロジックでも触るパーツ（矢印・ステージ名）は GetUIParts で個別に取得して、同じグループへ入れる。
			 * 情報パネルは StageInfoPanel が自分で持つ。
			 * 選択中だけ見せるパーツを足すときは、ここに追記する。
			 */
			constexpr uint32_t SELECTING_ONLY_PART_KEYS[] =
			{
				Hash32("BG"),
				Hash32("StageSelectBG"),
				Hash32("ButtonBG"),
				Hash32("SelectButton"),
				Hash32("SelectText"),
				Hash32("DecideButton"),
				Hash32("DecideText"),
				Hash32("BackButton"),
				Hash32("BackText"),
			};

			/** 矢印の入力ポップの長さ（秒）と大きさ（タイトル画面のカーソルと同じ手応えに揃える） */
			constexpr float ARROW_POP_DURATION = 0.15f;
			constexpr float ARROW_POP_SCALE = 0.2f;

			/** 矢印ポップのタイマー（m_arrowPopTimers）の添字 */
			constexpr size_t ARROW_LEFT = 0;
			constexpr size_t ARROW_RIGHT = 1;

			constexpr const char* STAGE_SELECT_JSON_PATH = "Assets/parameter/UI/stageSelect/StageSelect.json";
			constexpr const char* MENU_PARAM_KEY = "menuParam";
		}




		/************************************************************************************/


		StageSelectMenu::StageSelectMenu()
			: m_state(EnStageSelectState::Selecting)
			, m_selectingStage(EnStageChoices::Easy)
			, m_isVisibilityDirty(true)
			, m_leftArrow(nullptr)
			, m_rightArrow(nullptr)
			, m_stageNameText(nullptr)
			, m_stageNamePhase(EnStageNamePhase::Idle)
			, m_stageNameBaseScale(1.0f, 1.0f)
			, m_stageNameTargetIndex(0)
			, m_stagePreviewVideo(nullptr)
			, m_selectFlashIcon(nullptr)
			, m_prevSelectingStage(EnStageChoices::Max)
			, m_switchPhase(EnSwitchPhase::Idle)
			, m_arrowPopTimers()
			, m_selectEffectTimer(0.0f)
			, m_isZoomBaseCaptured(false)
			, m_isSelected(false)
		{}


		StageSelectMenu::~StageSelectMenu()
		{}


		void StageSelectMenu::InitializeLogic()
		{
			// Reload後に古いポインタが残らないようリセット
			m_selectingParts.clear();
			m_leftArrow = nullptr;
			m_rightArrow = nullptr;
			m_stageNameText = nullptr;
			m_stageNamePhase = EnStageNamePhase::Idle;
			m_switchPhase = EnSwitchPhase::Idle;
			m_stagePreviewVideo = nullptr;
			m_selectFlashIcon = nullptr;
			m_prevSelectingStage = EnStageChoices::Max;
			m_arrowPopTimers.fill(0.0f);

			// 選択確定演出の状態もリセットする
			m_zoomTargets.clear();
			m_isZoomBaseCaptured = false;
			m_selectEffectTimer = 0.0f;

			// Reload後に古い状態が残らないようリセットする。
			m_horizontalInputDetector.Reset();

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

			// この画面に入った最初のフレームは、このメニューのUpdateより先にRenderされることがある。
			// 更新前のレイアウトが映らないよう、最初のUpdateまでは隠しておく（表示はそこで反映する）
			ApplyVisibility(false);
			m_selectFlashIcon->SetIsDraw(false);
			m_isVisibilityDirty = true;
		}


		void StageSelectMenu::Update()
		{
			// 選択中だけ入力を受け付ける（選択確定後の見た目は UpdateSelectEffect が担当する）
			if (m_state == EnStageSelectState::Selecting)
			{
				UpdateSelecting();
				UpdateStageNameAnimation();
				UpdateSwitchTransition();
			}


			// 状態が変わった時にだけ、表示/非表示を反映する
			if (m_isVisibilityDirty)
			{
				m_isVisibilityDirty = false;
				ApplyVisibility(m_state == EnStageSelectState::Selecting);
			}
			UpdateIcons();

			// 選択確定演出（位置・スケールを上書きするので各Updateの後に行う）
			UpdateSelectEffect();

			// Canvasの更新
			MenuBase::Update();
		}


		void StageSelectMenu::SetState(const EnStageSelectState state)
		{
			m_state = state;
			m_isVisibilityDirty = true;
		}


		void StageSelectMenu::ApplyVisibility(const bool isShow)
		{
			for (auto* ui : m_selectingParts)
			{
				ui->SetIsDraw(isShow);
			}
			m_infoPanel.SetDraw(isShow);
		}


		void StageSelectMenu::CaptureZoomBase()
		{
			m_zoomTargets.clear();

			// ズームさせるのはステージ映像だけにする。
			// メニュー類（見出し・バブル・カーソル・ボタン）を一緒に拡大すると、
			// 文字やアイコンが画面外へ散っていくのが目に入って「ステージへ入っていく」感が薄れる。
			// それらは選択確定で ApplyVisibility() が演出の開始と同時に消す。
			// 白フラッシュは全画面のまま重ねたいので、ここでは対象にしない
			std::vector<UIBase*> targets = {
				m_stagePreviewVideo,
			};

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
				// 切り替え演出の途中で決定されても、暗転途中の色・スケールを基準値にしないよう先に終わらせる
				FinishSwitchTransition();
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
			SetState(EnStageSelectState::Selecting);
			m_selectingStage = EnStageChoices::Easy;
			m_prevSelectingStage = EnStageChoices::Max;
			m_arrowPopTimers.fill(0.0f);
			ResetStageNameAnimation();
			FinishSwitchTransition();

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

			m_isSelected = false;
		}


		void StageSelectMenu::UpdateSelecting()
		{
			// 選択済みになると状態を変更して抜ける
			if (m_isSelected)
			{
				SetState(EnStageSelectState::Selected);
				return;
			}


			const float stickLXF = g_pad[0]->GetLStickXF();

			// 横方向：Negative=左、Positive=右。倒しっぱなし中はinputIntervalごとにリピートする。
			const auto hDir = m_horizontalInputDetector.Update(
				stickLXF, g_pad[0]->IsTrigger(enButtonLeft), g_pad[0]->IsTrigger(enButtonRight),
				m_param.inputThreshold, m_param.inputInterval);
			const bool leftInput = hDir == Direction::Negative;
			const bool rightInput = hDir == Direction::Positive;



			// ステージ切り替えの手応え：SEと、押した側の矢印のポップ（タイトル画面のカーソルと同じ演出）
			auto PlayArrowSE = [&](const size_t arrow)
				{
					SoundManager::Get().PlaySE(static_cast<int>(enSoundKind::enSoundKind_CursorMove));
					m_arrowPopTimers[arrow] = ARROW_POP_DURATION;
				};

			// イージー・ノーマル・ハードの横移動
			const auto current = static_cast<uint8_t>(m_selectingStage);
			constexpr uint8_t HARD_INDEX = static_cast<uint8_t>(EnStageChoices::Hard);
			if (leftInput && current > 0)
			{
				m_selectingStage = static_cast<EnStageChoices>(current - 1);
				PlayArrowSE(ARROW_LEFT);
			}
			else if (rightInput && current < HARD_INDEX)
			{
				m_selectingStage = static_cast<EnStageChoices>(current + 1);
				PlayArrowSE(ARROW_RIGHT);
			}

			// ステージが変わったら、映像・ステージ名・情報パネルを切り替える。
			// 映像は事前ロード済みクリップにポインタを切り替える（I/O なし）
			if (m_selectingStage != m_prevSelectingStage)
			{
				// 画面に入って最初の反映は、切り替えではないので演出なしで表示する
				const bool isFirstApply = m_prevSelectingStage == EnStageChoices::Max;
				m_prevSelectingStage = m_selectingStage;
				const int stageIndex = static_cast<int>(m_selectingStage);
				if (isFirstApply)
				{
					ResetStageNameAnimation();
					m_stageNameText->SetText(m_param.stageNames.at(stageIndex));
					ApplyStageContent();
				}
				else
				{
					// 映像と情報パネルは、暗転の底で差し替える
					StartStageNameAnimation(stageIndex);
					StartSwitchTransition();
				}
			}
		}


		void StageSelectMenu::StartSwitchTransition()
		{
			// 暗くしている最中は、すでに暗転が進んでいるのでそのまま続ける（差し替え先は最新の m_selectingStage）
			if (m_switchPhase == EnSwitchPhase::FadeOut) return;

			// 明るく戻している最中なら今の明るさから、止まっているなら通常の明るさから暗くする（明るさが飛ばないように）
			const float startBrightness = (m_switchPhase == EnSwitchPhase::FadeIn)
				? m_switchCurve.GetCurrentValue()
				: 1.0f;
			m_switchCurve.Initialize(
				startBrightness, 0.0f, m_param.switchOutDuration, util::EasingType::Linear);
			m_switchCurve.Play();
			m_switchPhase = EnSwitchPhase::FadeOut;
		}


		void StageSelectMenu::UpdateSwitchTransition()
		{
			if (m_switchPhase == EnSwitchPhase::Idle) return;

			m_switchCurve.Update(g_gameTime->GetFrameDeltaTime());
			ApplySwitchLook(m_switchCurve.GetCurrentValue());

			// 今の段階が終わるまで待つ
			if (m_switchCurve.IsPlaying()) return;

			if (m_switchPhase == EnSwitchPhase::FadeOut)
			{
				// 真っ暗になったところで映像と情報パネルを差し替え、明るく戻す
				ApplyStageContent();
				m_switchCurve.Initialize(
					0.0f, 1.0f, m_param.switchInDuration, util::EasingType::EaseOut);
				m_switchCurve.Play();
				m_switchPhase = EnSwitchPhase::FadeIn;
			}
			else
			{
				m_switchPhase = EnSwitchPhase::Idle;
			}
		}


		void StageSelectMenu::ApplyStageContent()
		{
			const int stageIndex = static_cast<int>(m_selectingStage);
			if (m_stagePreviewVideo)
			{
				m_stagePreviewVideo->SwitchToPreloadedClip(stageIndex);
			}
			m_infoPanel.SetStage(stageIndex);
		}


		void StageSelectMenu::ApplySwitchLook(const float brightness)
		{
			// 映像：色を乗算して暗くし、暗い間だけ少し拡大する（明るく戻るにつれて元の大きさへ収まる）
			if (m_stagePreviewVideo)
			{
				m_stagePreviewVideo->m_color.x = brightness;
				m_stagePreviewVideo->m_color.y = brightness;
				m_stagePreviewVideo->m_color.z = brightness;
				const float scale = 1.0f + m_param.switchZoomScale * (1.0f - brightness);
				m_stagePreviewVideo->m_transform.m_localTransform.m_scale = Vector3(scale, scale, 1.0f);
			}

			// 情報パネル：映像と一緒に薄くする
			m_infoPanel.SetAlpha(brightness);
		}


		void StageSelectMenu::FinishSwitchTransition()
		{
			// 止まっているなら、見た目はすでに通常に戻っている
			if (m_switchPhase == EnSwitchPhase::Idle) return;

			// 差し替え前なら、先に最新のステージへ差し替える
			if (m_switchPhase == EnSwitchPhase::FadeOut)
			{
				ApplyStageContent();
			}
			m_switchPhase = EnSwitchPhase::Idle;
			ApplySwitchLook(1.0f);
		}


		void StageSelectMenu::StartStageNameAnimation(const int stageIndex)
		{
			// 差し替えるのは常に最新の難易度（拡大中に続けて切り替わっても、拡大後はこの難易度になる）
			m_stageNameTargetIndex = stageIndex;

			// 拡大中は、すでに拡大が進んでいるのでそのまま続ける
			if (m_stageNamePhase == EnStageNamePhase::Expanding) return;

			// 縮小中なら今の大きさから、止まっているなら元の大きさから拡大する（大きさが飛ばないように）
			const float startScale = (m_stageNamePhase == EnStageNamePhase::Shrinking)
				? m_stageNameCurve.GetCurrentValue()
				: 1.0f;
			m_stageNameCurve.Initialize(
				startScale, m_param.stageNamePopScale, m_param.stageNameExpandDuration, util::EasingType::EaseOut);
			m_stageNameCurve.Play();
			m_stageNamePhase = EnStageNamePhase::Expanding;
		}


		void StageSelectMenu::UpdateStageNameAnimation()
		{
			if (m_stageNamePhase == EnStageNamePhase::Idle) return;

			m_stageNameCurve.Update(g_gameTime->GetFrameDeltaTime());
			const float scale = m_stageNameCurve.GetCurrentValue();
			m_stageNameText->SetScale(Vector2(m_stageNameBaseScale.x * scale, m_stageNameBaseScale.y * scale));

			// 今の段階が終わるまで待つ
			if (m_stageNameCurve.IsPlaying()) return;

			if (m_stageNamePhase == EnStageNamePhase::Expanding)
			{
				// 拡大しきったところで次の難易度へ差し替える（拡大したまま登場して、縮小へ移る）
				m_stageNameText->SetText(m_param.stageNames.at(m_stageNameTargetIndex));
				m_stageNameCurve.Initialize(
					m_param.stageNamePopScale, 1.0f, m_param.stageNameShrinkDuration, util::EasingType::EaseOut);
				m_stageNameCurve.Play();
				m_stageNamePhase = EnStageNamePhase::Shrinking;
			}
			else
			{
				m_stageNamePhase = EnStageNamePhase::Idle;
			}
		}


		void StageSelectMenu::ResetStageNameAnimation()
		{
			m_stageNamePhase = EnStageNamePhase::Idle;
			if (m_stageNameText)
			{
				m_stageNameText->SetScale(m_stageNameBaseScale);
			}
		}


		void StageSelectMenu::UpdateIcons()
		{
			auto UpdateArrow = [&](UIIcon* arrow, const size_t index, const bool isMovable)
				{
					// 入力のポップ（矢印が一瞬大きくなって戻る）
					if (m_arrowPopTimers[index] > 0.0f)
					{
						m_arrowPopTimers[index] -= g_gameTime->GetFrameDeltaTime();
					}
					const float pop = 1.0f
						+ ARROW_POP_SCALE * (std::max)(m_arrowPopTimers[index], 0.0f) / ARROW_POP_DURATION;
					arrow->m_transform.m_localTransform.m_scale = Vector3(pop, pop, 1.0f);

					// 端のステージでは、動かせない側の矢印を薄くする
					arrow->m_color.w = isMovable ? 1.0f : m_param.arrowDisabledAlpha;
				};

			UpdateArrow(m_leftArrow, ARROW_LEFT, m_selectingStage != EnStageChoices::Easy);
			UpdateArrow(m_rightArrow, ARROW_RIGHT, m_selectingStage != EnStageChoices::Hard);
		}


		void StageSelectMenu::GetUIParts()
		{
			// 選択中だけ見せるグループへ登録する（取得できていなければアサート）
			auto AddSelectingPart = [&](UIBase* ui)
				{
					K2_ASSERT(ui, "UIパーツを取得できていません。");
					m_selectingParts.push_back(ui);
				};

			// 表示/非表示にしか使わないパーツ
			for (const uint32_t key : SELECTING_ONLY_PART_KEYS)
			{
				AddSelectingPart(GetUI<UIBase>(key));
			}

			// ロジックでも触るパーツは型付きで持ち、同じグループへも入れる
			m_leftArrow = GetUI<UIIcon>(Hash32("LeftArrow"));
			AddSelectingPart(m_leftArrow);
			m_rightArrow = GetUI<UIIcon>(Hash32("RightArrow"));
			AddSelectingPart(m_rightArrow);
			m_stageNameText = GetUI<UIText>(Hash32("StageNameText"));
			AddSelectingPart(m_stageNameText);
			// 切り替え演出で拡縮するので、元の大きさ（JSONの値）を覚えておく
			m_stageNameBaseScale = m_stageNameText->GetScale();

			// 情報パネルは自分でパーツを持つ
			m_infoPanel.Initialize(*this);

			// 映像と白フラッシュは、選択確定演出が個別に制御するのでグループに入れない
			m_stagePreviewVideo = GetUI<UIVideo>(Hash32("StagePreviewVideo"));

			m_selectFlashIcon = GetUI<UIIcon>(Hash32("SelectFlashWhite"));
			K2_ASSERT(m_selectFlashIcon, "アイコンを取得できていません。");
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
			m_param.arrowDisabledAlpha = JC::ToFloat(p, "arrowDisabledAlpha", m_param.arrowDisabledAlpha);
			m_param.switchOutDuration = JC::ToFloat(p, "switchOutDuration", m_param.switchOutDuration);
			m_param.switchInDuration = JC::ToFloat(p, "switchInDuration", m_param.switchInDuration);
			m_param.switchZoomScale = JC::ToFloat(p, "switchZoomScale", m_param.switchZoomScale);
			m_param.stageNamePopScale = JC::ToFloat(p, "stageNamePopScale", m_param.stageNamePopScale);
			m_param.stageNameExpandDuration = JC::ToFloat(p, "stageNameExpandDuration", m_param.stageNameExpandDuration);
			m_param.stageNameShrinkDuration = JC::ToFloat(p, "stageNameShrinkDuration", m_param.stageNameShrinkDuration);

			if (p.contains("stageNames") && p["stageNames"].is_array())
			{
				const auto& names = p["stageNames"];
				for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max) && i < names.size(); ++i)
				{
					m_param.stageNames[i] = names[i].get<std::string>();
				}
			}

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
