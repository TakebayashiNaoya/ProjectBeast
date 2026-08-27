/**
 * @file TitleEventMenu.cpp
 * @brief タイトルの動的処理クラス
 */
#include "stdafx.h"
#include "TitleEventMenu.h"
#include "UIMenuConstants.h"

#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr std::array<uint32_t, static_cast<uint8_t>(TitleEventMenu::EnEventType::Num)> EVENT_ICON_KEYS =
			{
				Hash32("StartIcon"),
				Hash32("SoundIcon"),
				Hash32("RuleIcon"),
				Hash32("EndIcon")
			};

			//============================================//
			// タイトルの環境演出（音符・雪・ロゴ弾み）
			//============================================//

			/** 音符・雪の個数（Title.jsonの要素数と一致させること） */
			constexpr int NOTE_NUM = 6;
			constexpr int SNOW_NUM = 12;

			/** ロゴの弾み：1拍の周波数（Hz）と振幅。速いと落ち着かないため半拍相当でゆったり弾ませる */
			constexpr float ROGO_BEAT_HZ = 0.7f;
			constexpr float ROGO_BOUNCE_AMPLITUDE = 0.022f;

			/** 音符：湧き上がりの1周期（秒・個体でずらす）、上昇距離、横揺れ幅 */
			constexpr float NOTE_CYCLE_BASE = 4.2f;
			constexpr float NOTE_CYCLE_STEP = 0.55f;
			constexpr float NOTE_RISE_HEIGHT = 260.0f;
			constexpr float NOTE_SWAY_WIDTH = 26.0f;
			/** 音符の湧き位置（ロゴ周辺に散らす） */
			constexpr float NOTE_BASE_X[NOTE_NUM] = { -430.0f, -260.0f, -80.0f, 120.0f, 300.0f, 450.0f };
			constexpr float NOTE_BASE_Y = -40.0f;

			/** 雪：落下の1周期（秒・個体でずらす）、落下距離、横揺れ幅 */
			constexpr float SNOW_CYCLE_BASE = 7.0f;
			constexpr float SNOW_CYCLE_STEP = 0.9f;
			constexpr float SNOW_FALL_HEIGHT = 1000.0f;
			constexpr float SNOW_SWAY_WIDTH = 40.0f;
			constexpr float SNOW_TOP_Y = 480.0f;

			/** カーソル移動ポップの長さ（秒）と大きさ */
			constexpr float CURSOR_POP_DURATION = 0.15f;
			constexpr float CURSOR_POP_SCALE = 0.2f;

			/** インデックスから疑似乱数的な位相（0〜1）を作る */
			float PhaseOf(int index)
			{
				return fmodf(static_cast<float>(index) * 0.618034f, 1.0f);
			}
		}
		TitleEventMenu::TitleEventMenu()
			: m_isSelect(false)
			, m_isStickNeutral(true)
			, m_isDraw(false)
			, m_selectIndex(TitleEventMenu::EnEventType::Start)
			, m_gamePad(g_pad[0])
			, m_bgIcon(nullptr)
			, m_rogoIcon(nullptr)
			, m_frameIcon(nullptr)
			, m_frameBackIcon(nullptr)
			, m_eventIcon{ nullptr }
			, m_axisInputDetector()
			, m_cursorSelector(static_cast<int>(EnEventType::Num))
		{}


		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			const float stickX = m_gamePad->GetLStickXF();

			// 右入力（十字キーまたはスティック右）でPositive、左入力でNegative。
			const auto dir = m_axisInputDetector.Update(
				stickX, m_gamePad->IsTrigger(enButtonLeft), m_gamePad->IsTrigger(enButtonRight), STICK_THRESHOLD);

			if (m_cursorSelector.TryMove(dir))
			{
				m_selectIndex = static_cast<EnEventType>(m_cursorSelector.Get());
				SelectVisual();

				// カーソル移動の手応え：SEとフレームのポップ
				SoundManager::Get().PlaySE(enSoundKind_CursorMove, 1.0f);
				m_cursorPopTimer = CURSOR_POP_DURATION;
			}

			UpdateDrawFlag();
			UpdateAmbient();
			MenuBase::Update();
		}


		void TitleEventMenu::UpdateAmbient()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			m_ambientTimer += deltaTime;
			const float t = m_ambientTimer;

			// ロゴが拍でぽよんと弾む（音楽モチーフのタイトルなので拍感を出す）
			if (m_rogoIcon)
			{
				if (!m_isRogoBaseScaleCaptured)
				{
					m_rogoBaseScale = m_rogoIcon->m_transform.m_localTransform.m_scale;
					m_isRogoBaseScaleCaptured = true;
				}
				const float bounce = fabsf(sinf(Math::PI * t * ROGO_BEAT_HZ));
				const float scale = 1.0f + ROGO_BOUNCE_AMPLITUDE * bounce;
				m_rogoIcon->m_transform.m_localTransform.m_scale =
					Vector3(m_rogoBaseScale.x * scale, m_rogoBaseScale.y * scale, m_rogoBaseScale.z);
			}

			// 音符がロゴ周辺から湧き上がる（フェードイン→上昇→フェードアウトのループ）
			for (int i = 0; i < NOTE_NUM; ++i)
			{
				char name[32];
				sprintf_s(name, "TitleNote%d", i);
				auto* note = GetUI<UIIcon>(Hash32(name));
				if (note == nullptr) continue;

				const float cycle = NOTE_CYCLE_BASE + NOTE_CYCLE_STEP * i;
				float u = t / cycle + PhaseOf(i);
				u -= floorf(u);

				const float x = NOTE_BASE_X[i]
					+ NOTE_SWAY_WIDTH * sinf(t * 1.3f + PhaseOf(i) * 6.28f);
				const float y = NOTE_BASE_Y + NOTE_RISE_HEIGHT * u;

				/** 出現直後と消える前をフェードさせる（0→0.15でイン、0.6→1でアウト） */
				const float fadeIn = (std::min)(u / 0.15f, 1.0f);
				const float fadeOut = (std::min)((1.0f - u) / 0.4f, 1.0f);

				note->SetIsDraw(true);
				note->m_transform.m_localTransform.m_position = Vector3(x, y, 0.0f);
				note->m_color.w = 0.85f * fadeIn * fadeOut;
			}

			// 雪がゆっくり降る
			for (int i = 0; i < SNOW_NUM; ++i)
			{
				char name[32];
				sprintf_s(name, "TitleSnow%d", i);
				auto* snow = GetUI<UIIcon>(Hash32(name));
				if (snow == nullptr) continue;

				const float cycle = SNOW_CYCLE_BASE + SNOW_CYCLE_STEP * (i % 5);
				float u = t / cycle + PhaseOf(i + 7);
				u -= floorf(u);

				const float x = (PhaseOf(i) * 2.0f - 1.0f) * 780.0f
					+ SNOW_SWAY_WIDTH * sinf(t * 0.8f + PhaseOf(i) * 6.28f);
				const float y = SNOW_TOP_Y - SNOW_FALL_HEIGHT * u;

				snow->SetIsDraw(true);
				snow->m_transform.m_localTransform.m_position = Vector3(x, y, 0.0f);
			}

			// カーソル移動のポップ（フレームが一瞬大きくなって戻る）
			if (m_cursorPopTimer > 0.0f)
			{
				m_cursorPopTimer -= deltaTime;
			}
			const float pop = 1.0f
				+ CURSOR_POP_SCALE * (std::max)(m_cursorPopTimer, 0.0f) / CURSOR_POP_DURATION;
			if (m_frameIcon)
			{
				m_frameIcon->m_transform.m_localTransform.m_scale = Vector3(pop, pop, 1.0f);
			}
			if (m_frameBackIcon)
			{
				m_frameBackIcon->m_transform.m_localTransform.m_scale = Vector3(pop, pop, 1.0f);
			}
		}


		void TitleEventMenu::InitializeLogic()
		{
			// Reload後に古いポインタが残らないようリセット
			m_bgIcon = nullptr;
			m_rogoIcon = nullptr;
			m_frameIcon = nullptr;
			m_frameBackIcon = nullptr;
			m_eventIcon.fill(nullptr);

			// UIパーツを取得
			GetUIParts();

			// 最初は全て非表示
			if (m_bgIcon) m_bgIcon->SetIsDraw(false);

			if (m_rogoIcon) m_rogoIcon->SetIsDraw(false);

			if (m_frameIcon) m_frameIcon->SetIsDraw(false);

			if (m_frameBackIcon) m_frameBackIcon->SetIsDraw(false);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(false);
			}

			// 入力検出・カーソル選択の状態をリセット。
			m_axisInputDetector.Reset();
			m_cursorSelector.Reset();
			m_selectIndex = static_cast<EnEventType>(m_cursorSelector.Get());

			// 最初の選択状態を設定。
			SelectVisual();
		}


		void TitleEventMenu::SelectVisual()
		{
			auto* icon = m_frameBackIcon;
			// アイコンがない場合は処理しない。
			if (!icon || !m_frameIcon) return;


			constexpr uint32_t animKey = Hash32("EventColorAnim");

			// フレームの背景アイコンからアニメーションを取り除く。
			if (!icon->FindAnimation(animKey))
			{
				// アニメーションのパラメーター
				const Vector4 startColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				const Vector4 endColor = Vector4(1.0f, 1.0f, 1.0f, 0.2f);
				constexpr float duration = 0.6f;

				auto colorAnim = std::make_unique<UIColorAnimation>();
				colorAnim->SetParameter(
					startColor
					, endColor
					, duration
					, util::EasingType::EaseOut
					, util::LoopMode::PingPong
				);

				// アイコンにアニメーションを登録。
				icon->AddAnimation(animKey, std::move(colorAnim));
				icon->PlayAnimation();
			}

			// 選択されているアイコンを取得。
			auto* eventIcon = m_eventIcon.at(static_cast<uint8_t>(m_selectIndex));
			// アイコンがない場合は処理しない。
			if (!eventIcon) return;

			// アイコンの位置をフレームの背景アイコンの位置に合わせる。
			const Vector3 eventIconPosition = eventIcon->m_transform.m_localTransform.m_position;
			icon->m_transform.m_localTransform.m_position = eventIconPosition;
			m_frameIcon->m_transform.m_localTransform.m_position = eventIconPosition;
		}


		uint32_t TitleEventMenu::GetSelectKey()const
		{
			return EVENT_ICON_KEYS[static_cast<int>(m_selectIndex)];
		}



		void TitleEventMenu::GetUIParts()
		{
			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("TitleBackGround"));
			if (!m_rogoIcon) m_rogoIcon = GetUI<UIIcon>(Hash32("PentaktRogoIcon"));
			if (!m_frameIcon) m_frameIcon = GetUI<UIIcon>(Hash32("Frame"));
			if (!m_frameBackIcon) m_frameBackIcon = GetUI<UIIcon>(Hash32("FrameBack"));

			for (int i = 0; i < static_cast<int>(EnEventType::Num); i++)
			{
				if (!m_eventIcon.at(i))
				{
					m_eventIcon.at(i) = GetUI<UIText>(EVENT_ICON_KEYS.at(i));
				}
			}
		}

		void TitleEventMenu::UpdateDrawFlag()
		{
			GetUIParts();
			if (m_bgIcon) m_bgIcon->SetIsDraw(true);
			if (m_rogoIcon) m_rogoIcon->SetIsDraw(true);
			if (m_frameIcon) m_frameIcon->SetIsDraw(true);
			if (m_frameBackIcon) m_frameBackIcon->SetIsDraw(true);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(true);
			}
		}
	}
}