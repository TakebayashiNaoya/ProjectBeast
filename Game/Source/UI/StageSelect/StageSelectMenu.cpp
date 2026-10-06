/**
 * @file StageSelectMenu.cpp
 * @brief ステージ選択画面のメニュークラス
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
			/** ステージ選択肢のバブル（Easy/Normal/Hard順）。カーソルの位置合わせに使う */
			constexpr uint32_t BUBBLE_KEYS[] =
			{
				Hash32("EasyBubble"),
				Hash32("NormalBubble"),
				Hash32("HardBubble"),
			};
			static_assert(std::size(BUBBLE_KEYS) == static_cast<size_t>(EnStageChoices::Max),
				"バブルの数と EnStageChoices の件数を揃えること");

			/**
			 * 選択中だけ見せるパーツのうち、表示/非表示にしか使わないもの。
			 * ロジックでも触るパーツ（バブル・カーソル）は GetUIParts で個別に取得して、同じグループへ入れる。
			 * 情報パネルは StageInfoPanel が自分で持つ。
			 * 選択中だけ見せるパーツを足すときは、ここに追記する。
			 */
			constexpr uint32_t SELECTING_ONLY_PART_KEYS[] =
			{
				Hash32("BG"),
				Hash32("StageSelectBG"),
				Hash32("StageSelectText"),
				Hash32("EasyText"),
				Hash32("NormalText"),
				Hash32("HardText"),
				Hash32("ButtonBG"),
				Hash32("SelectButton"),
				Hash32("SelectText"),
				Hash32("DecideButton"),
				Hash32("DecideText"),
				Hash32("BackButton"),
				Hash32("BackText"),
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


		StageSelectMenu::StageSelectMenu()
		{}


		StageSelectMenu::~StageSelectMenu()
		{}


		void StageSelectMenu::InitializeLogic()
		{
			// Reload後に古いポインタが残らないようリセット
			m_selectingParts.clear();
			m_bubbleIcons.fill(nullptr);
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
				SetState(EnStageSelectState::Selected);
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

			// 横方向：Negative=左、Positive=右。倒しっぱなし中はinputIntervalごとにリピートする。
			const auto hDir = m_horizontalInputDetector.Update(
				stickLXF, g_pad[0]->IsTrigger(enButtonLeft), g_pad[0]->IsTrigger(enButtonRight),
				m_param.inputThreshold, m_param.inputInterval);
			const bool leftInput = hDir == Direction::Negative;
			const bool rightInput = hDir == Direction::Positive;



			// カーソル移動の手応え：SEとフレームのポップ（タイトル画面と同じ演出）
			auto PlayCursorSE = [&]()
				{
					SoundManager::Get().PlaySE(static_cast<int>(enSoundKind::enSoundKind_CursorMove));
					m_cursorPopTimer = CURSOR_POP_DURATION;
				};

			// イージー・ノーマル・ハードの横移動
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

			// ステージが変わったら、映像と情報パネルを切り替える。
			// 映像は事前ロード済みクリップにポインタを切り替える（I/O なし）
			if (m_selectingStage != m_prevSelectingStage)
			{
				m_prevSelectingStage = m_selectingStage;
				const int stageIndex = static_cast<int>(m_selectingStage);
				if (m_stagePreviewVideo)
				{
					m_stagePreviewVideo->SwitchToPreloadedClip(stageIndex);
				}
				m_infoPanel.SetStage(stageIndex);
			}
		}


		void StageSelectMenu::UpdateSelected()
		{
			// 万が一選択されていない状態でここに来ると抜ける
			if (!m_isSelected) return;

			if (m_cursorFrameBG->IsPlayAnimation()) m_cursorFrameBG->StopAnimation();
		}


		void StageSelectMenu::UpdateIcons()
		{
			// カーソルの位置を選択中のバブルに合わせる
			const auto* selected = m_bubbleIcons.at(static_cast<uint8_t>(m_selectingStage));
			const Vector3 position = selected->m_transform.m_localTransform.m_position;
			m_cursorFrame->m_transform.m_localTransform.m_position = position;
			m_cursorFrameBG->m_transform.m_localTransform.m_position = position;

			// カーソル移動のポップ（フレームが一瞬大きくなって戻る）
			if (m_cursorPopTimer > 0.0f)
			{
				m_cursorPopTimer -= g_gameTime->GetFrameDeltaTime();
			}
			const float pop = 1.0f
				+ CURSOR_POP_SCALE * (std::max)(m_cursorPopTimer, 0.0f) / CURSOR_POP_DURATION;

			const Vector3 cursorScale = Vector3(pop, pop, 1.0f);
			m_cursorFrame->m_transform.m_localTransform.m_scale = cursorScale;
			m_cursorFrameBG->m_transform.m_localTransform.m_scale = cursorScale;
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
			for (uint8_t i = 0; i < static_cast<uint8_t>(EnStageChoices::Max); ++i)
			{
				m_bubbleIcons.at(i) = GetUI<UIIcon>(BUBBLE_KEYS[i]);
				AddSelectingPart(m_bubbleIcons.at(i));
			}

			m_cursorFrame = GetUI<UIIcon>(Hash32("Frame"));
			AddSelectingPart(m_cursorFrame);
			m_cursorFrameBG = GetUI<UIIcon>(Hash32("FrameBG"));
			AddSelectingPart(m_cursorFrameBG);

			// 情報パネルは自分でパーツを持つ
			m_infoPanel.Initialize(*this);

			// 映像と白フラッシュは、選択確定演出が個別に制御するのでグループに入れない
			m_stagePreviewVideo = GetUI<UIVideo>(Hash32("StagePreviewVideo"));

			m_selectFlashIcon = GetUI<UIIcon>(Hash32("SelectFlashWhite"));
			K2_ASSERT(m_selectFlashIcon, "アイコンを取得できていません。");
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
