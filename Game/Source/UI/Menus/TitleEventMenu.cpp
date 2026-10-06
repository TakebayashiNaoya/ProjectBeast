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


			/** 音符のパーティクルエフェクト用のJsonファイルパス */
			constexpr const char* MUSICAL_NOTE_EFFECT_JSON_PATH = "Assets/parameter/UI/vfx/musicalnoteEffect/musicalNoteEffect.json";
			/** 音符のテクスチャファイルパス */
			constexpr const char* MUSICAL_NOTE_EFFECT_TEXTURE_PATH = "Assets/effect/vfx/MusicalNoteTexture/musicalNoteTexture.dds";
			/** 音符テクスチャ1つの大きさ */
			constexpr float MUSICAL_NOTE_SPRITE_SIZE = 40.0f;
			/** 音符の最大スケール（musicalNoteEffect.jsonのScaleOverLifeのendScaleの最大値と合わせること） */
			constexpr float MUSICAL_NOTE_MAX_SCALE = 1.5f;
			/** 音符を出す帯の幅（ロゴに被らない位置から外側へ広げる量。小さいほどロゴの近くに出る） */
			constexpr float MUSICAL_NOTE_AREA_WIDTH = 80.0f;
			/** 音符を出す帯の高さの割合（ロゴの中心から画面上端の限界までのうち、下からこの割合までに出す） */
			constexpr float MUSICAL_NOTE_AREA_HEIGHT_RATE = 0.5f;
			/** 画面の半分の大きさ（音符が画面からはみ出さないようにするため） */
			constexpr float SCREEN_HALF_WIDTH = FRAME_BUFFER_W * 0.5f;
			constexpr float SCREEN_HALF_HEIGHT = FRAME_BUFFER_H * 0.5f;

			//============================================//
			// タイトルの環境演出（音符・雪・ロゴ弾み）
			//============================================//

			/** 雪の個数（Title.jsonの要素数と一致させること） */
			constexpr int SNOW_NUM = 12;

			/** ロゴの弾み：1拍の周波数（Hz）と振幅。速いと落ち着かないため半拍相当でゆったり弾ませる */
			constexpr float ROGO_BEAT_HZ = 0.7f;
			constexpr float ROGO_BOUNCE_AMPLITUDE = 0.022f;

			/** 雪：落下の1周期（秒・個体でずらす）、落下距離、横揺れ幅 */
			constexpr float SNOW_CYCLE_BASE = 7.0f;
			constexpr float SNOW_CYCLE_STEP = 0.9f;
			constexpr float SNOW_FALL_HEIGHT = 1000.0f;
			constexpr float SNOW_SWAY_WIDTH = 40.0f;
			constexpr float SNOW_TOP_Y = 480.0f;

			/** カーソル移動ポップの長さ（秒）と大きさ */
			constexpr float CURSOR_POP_DURATION = 0.15f;
			constexpr float CURSOR_POP_SCALE = 0.15f;

			/** 選択中の項目の文字の大きさ（JSON上の大きさに対する倍率） */
			constexpr float SELECTED_TEXT_SCALE = 1.25f;
			/** 選択していない項目の文字の不透明度（JSON上の不透明度に対する倍率） */
			constexpr float UNSELECTED_TEXT_ALPHA = 0.5f;

			/** 文字1つ分の横幅（文字の大きさ1.0のとき）。文字列の横幅を文字数から見積もるのに使う */
			constexpr float TEXT_CHAR_WIDTH = 46.0f;
			/** 選択中の文字と音符アイコンの隙間 */
			constexpr float SELECT_NOTE_GAP = 12.0f;
			/** 音符アイコンの拡縮の振れ幅（拡縮の頻度はロゴの弾みと同じROGO_BEAT_HZを使う。1.0〜1.0+この値の倍率で拡縮する） */
			constexpr float SELECT_NOTE_PULSE_AMPLITUDE = 0.15f;

			/** 文字1つ分の高さ（文字の大きさ1.0のとき）。下線を置く高さを見積もるのに使う */
			constexpr float TEXT_CHAR_HEIGHT = 46.0f;
			/** 選択中の文字と下線の隙間 */
			constexpr float SELECT_UNDERLINE_GAP = 4.0f;

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
			, m_selectNoteIcon(nullptr)
			, m_selectUnderlineIcon(nullptr)
			, m_eventIcon{ nullptr }
			, m_eventBaseScales{ Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f) }
			, m_eventBaseColors{ Vector4::White, Vector4::White, Vector4::White, Vector4::White }
			, m_axisInputDetector()
			, m_cursorSelector(static_cast<int>(EnEventType::Num))
			, m_musicalNoteEffectRenders()
			, m_isMusicalNoteEffectInitialized(false)
		{}


		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			const float stickY = m_gamePad->GetLStickYF();

			// 項目は上から縦に並んでいるので、下入力（十字キーまたはスティック下）でPositive（次の項目）、
			// 上入力でNegative（前の項目）。端まで行くと反対の端に戻る。
			const auto dir = m_axisInputDetector.Update(
				-stickY, m_gamePad->IsTrigger(enButtonUp), m_gamePad->IsTrigger(enButtonDown), STICK_THRESHOLD);

			if (m_cursorSelector.TryMove(dir))
			{
				m_selectIndex = static_cast<EnEventType>(m_cursorSelector.Get());

				// カーソル移動の手応え：SEと選択した文字のポップ
				SoundManager::Get().PlaySE(enSoundKind_CursorMove, 1.0f);
				m_cursorPopTimer = CURSOR_POP_DURATION;
				// 前の項目の拡縮の途中経過は引き継がず、新しい項目の音符は元の大きさから拡縮を始める
				m_selectNotePulseTimer = 0.0f;
				SelectVisual();
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

			// 音符のパーティクルエフェクトを更新（発生範囲をロゴに合わせてから）
			UpdateMusicalNoteSpawnArea();
			for (auto& effectRender : m_musicalNoteEffectRenders)
			{
				effectRender.Update(deltaTime);
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

			// カーソル移動のポップ（選択した文字が一瞬大きくなって戻る）
			if (m_cursorPopTimer > 0.0f)
			{
				m_cursorPopTimer -= deltaTime;
			}
			// 音符アイコンの拡縮を進める
			m_selectNotePulseTimer += deltaTime;
			SelectVisual();
		}


		void TitleEventMenu::UpdateMusicalNoteSpawnArea()
		{
			if (!m_rogoIcon) return;

			const Vector3& rogoPosition = m_rogoIcon->m_transform.m_localTransform.m_position;

			// ロゴが一番大きく弾んだときの表示サイズの半分
			const Vector3& scale = m_isRogoBaseScaleCaptured
				? m_rogoBaseScale
				: m_rogoIcon->m_transform.m_localTransform.m_scale;
			const Vector2& size = m_rogoIcon->GetSize();
			const float bounceScale = 1.0f + ROGO_BOUNCE_AMPLITUDE;
			const float rogoHalfWidth = size.x * scale.x * bounceScale * 0.5f;
			const float rogoHalfHeight = size.y * scale.y * bounceScale * 0.5f;

			// 音符が一番大きくなったときの半分の大きさ
			const float noteHalfSize = MUSICAL_NOTE_SPRITE_SIZE * MUSICAL_NOTE_MAX_SCALE * 0.5f;

			// くり抜く範囲：音符の中心がここより外なら、音符の端もロゴに被らない
			const Vector2 innerHalfSize(rogoHalfWidth + noteHalfSize, rogoHalfHeight + noteHalfSize);

			// 音符の中心が画面からはみ出さない限界（エミッターからの相対座標）
			const float leftLimit = -(SCREEN_HALF_WIDTH - noteHalfSize) - rogoPosition.x;
			const float topLimit = (SCREEN_HALF_HEIGHT - noteHalfSize) - rogoPosition.y;

			const float rightLimit = (SCREEN_HALF_WIDTH - noteHalfSize) - rogoPosition.x;

			// 帯の上端：画面上端すれすれに出ないよう、ロゴの中心から画面上端の限界までの高さを割合で縮める
			const float areaTop = topLimit * MUSICAL_NOTE_AREA_HEIGHT_RATE;

			// ロゴを真ん中として、ロゴの左側と右側の縦長の帯に4個ずつ出す
			// 下に行かないよう、高さはどちらもロゴの中心から帯の上端まで
			// [0]:ロゴの左側の帯、[1]:ロゴの右側の帯（画面端までの空きが少ない分だけ細くなる）
			const std::array<Vector2, 2> outerMins =
			{
				Vector2((std::max)(-innerHalfSize.x - MUSICAL_NOTE_AREA_WIDTH, leftLimit), 0.0f),
				Vector2(innerHalfSize.x, 0.0f)
			};
			const std::array<Vector2, 2> outerMaxs =
			{
				Vector2(-innerHalfSize.x, areaTop),
				Vector2((std::min)(innerHalfSize.x + MUSICAL_NOTE_AREA_WIDTH, rightLimit), areaTop)
			};

			for (size_t i = 0; i < m_musicalNoteEffectRenders.size(); ++i)
			{
				// エミッターをロゴの中心に置く
				ParticleEffectRender& effectRender = m_musicalNoteEffectRenders[i];
				effectRender.SetPosition(rogoPosition);

				// JSONに無い、またはホットリロードで作り直された場合でも発生範囲を持たせる
				ParticleEmitter& emitter = effectRender.GetEmitter();
				InitPositionOutsideRectModule* positionModule = emitter.FindModule<InitPositionOutsideRectModule>();
				if (!positionModule)
				{
					positionModule = emitter.AddModule<InitPositionOutsideRectModule>();
					// 左右どちらのグループの音符とも重ならない位置にだけ出す
					positionModule->SetBlockedChecker([this](const Vector2& localPosition)
						{
							return IsMusicalNoteOverlapped(localPosition);
						});
				}
				// エフェクトごとに帯を分けて、それぞれ同じ数ずつ出す
				positionModule->SetArea(outerMins[i], outerMaxs[i], innerHalfSize);
				positionModule->SetAllowedQuadrants(InitPositionOutsideRectModule::QuadrantAll);
			}
		}


		bool TitleEventMenu::IsMusicalNoteOverlapped(const Vector2& localPosition)
		{
			// 音符が一番大きくなったときの大きさで、四角形同士の重なりを判定する
			const float noteSize = MUSICAL_NOTE_SPRITE_SIZE * MUSICAL_NOTE_MAX_SCALE;

			for (auto& effectRender : m_musicalNoteEffectRenders)
			{
				ParticleEmitter& emitter = effectRender.GetEmitter();
				const Vector3& origin = emitter.GetPosition();
				for (const Particle& particle : emitter.GetParticles())
				{
					if (!particle.isAlive) continue;

					// 出ている音符の位置をエミッターからの相対座標にして比べる
					const float dx = particle.position.x - origin.x - localPosition.x;
					const float dy = particle.position.y - origin.y - localPosition.y;
					if (fabsf(dx) < noteSize && fabsf(dy) < noteSize)
					{
						return true;
					}
				}
			}
			return false;
		}


		void TitleEventMenu::Render(RenderContext& rc)
		{
			MenuBase::Render(rc);

			if (m_isMusicalNoteEffectInitialized)
			{
				// 音符のパーティクルエフェクトを描画
				for (auto& effectRender : m_musicalNoteEffectRenders)
				{
					effectRender.Draw(rc);
				}
			}
		}


		void TitleEventMenu::InitializeLogic()
		{
			// Reload後に古いポインタが残らないようリセット
			m_bgIcon = nullptr;
			m_rogoIcon = nullptr;
			m_selectNoteIcon = nullptr;
			m_selectUnderlineIcon = nullptr;
			m_eventIcon.fill(nullptr);
			m_isEventBaseVisualCaptured = false;
			m_cursorPopTimer = 0.0f;
			m_selectNotePulseTimer = 0.0f;

			// UIパーツを取得
			GetUIParts();

			// 選択演出で書き換える前に、項目の文字のJSON上の大きさと色を記録しておく
			CaptureEventBaseVisual();

			// 最初は全て非表示
			if (m_bgIcon) m_bgIcon->SetIsDraw(false);

			if (m_rogoIcon) m_rogoIcon->SetIsDraw(false);

			if (m_selectNoteIcon) m_selectNoteIcon->SetIsDraw(false);

			if (m_selectUnderlineIcon) m_selectUnderlineIcon->SetIsDraw(false);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(false);
			}

			if (!m_isMusicalNoteEffectInitialized)
			{
				/**
				 * @brief 音符のパーティクルエフェクトを初期化
				 * @details : 上からJSONパス、Textureパス(.dds)、横幅、縦幅
				 */
				for (auto& effectRender : m_musicalNoteEffectRenders)
				{
					effectRender.Init(
						MUSICAL_NOTE_EFFECT_JSON_PATH,
						MUSICAL_NOTE_EFFECT_TEXTURE_PATH,
						MUSICAL_NOTE_SPRITE_SIZE,
						MUSICAL_NOTE_SPRITE_SIZE
					);
#ifdef _DEBUG
					// 手動でパーティクルをホットリロードにかける
					effectRender.EnableHotReload();
#endif
				}
				m_isMusicalNoteEffectInitialized = true;
			}

			// 音符のパーティクルエフェクトを最初から再生
			for (auto& effectRender : m_musicalNoteEffectRenders)
			{
				effectRender.Reset();
				effectRender.Play();
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
			// 基準の大きさと色が分からないうちは書き換えない
			if (!CaptureEventBaseVisual()) return;

			// カーソル移動直後は選択した文字を一瞬大きくする
			const float pop = 1.0f
				+ CURSOR_POP_SCALE * (std::max)(m_cursorPopTimer, 0.0f) / CURSOR_POP_DURATION;

			for (size_t i = 0; i < m_eventIcon.size(); ++i)
			{
				UIText* text = m_eventIcon[i];
				const bool isSelected = (i == static_cast<size_t>(m_selectIndex));

				// 選択中は大きく、それ以外はJSON上の大きさ
				const float scaleRate = isSelected ? SELECTED_TEXT_SCALE * pop : 1.0f;
				text->SetScale(Vector2(m_eventBaseScales[i].x * scaleRate, m_eventBaseScales[i].y * scaleRate));

				// 選択中はJSON上の色、それ以外は半透明
				Vector4 color = m_eventBaseColors[i];
				if (!isSelected)
				{
					color.w *= UNSELECTED_TEXT_ALPHA;
				}
				text->m_color = color;

				if (!isSelected) continue;

				// 選択中の文字の横幅と高さの半分（文字数と文字の大きさから見積もる）
				const float textHalfWidth =
					TEXT_CHAR_WIDTH * static_cast<float>(text->GetTextLength()) * text->GetScale().x * 0.5f;
				const float textHalfHeight = TEXT_CHAR_HEIGHT * text->GetScale().y * 0.5f;
				const Vector3& textPosition = text->m_transform.m_localTransform.m_position;

				// 選択中の文字の左横に音符アイコンを置き、表示している間は拡縮させる（カーソル移動直後はポップも重ねる）
				if (m_selectNoteIcon)
				{
					// ロゴの弾みと同じ式・同じ頻度で拡縮させる
					const float pulse = 1.0f + SELECT_NOTE_PULSE_AMPLITUDE
						* fabsf(sinf(Math::PI * ROGO_BEAT_HZ * m_selectNotePulseTimer));
					const float noteScale = pop * pulse;
					// 拡縮しても文字に近づきすぎないよう、一番大きくなったときの大きさで位置を決める
					const float noteMaxHalfWidth = m_selectNoteIcon->GetSize().x * pop * (1.0f + SELECT_NOTE_PULSE_AMPLITUDE) * 0.5f;
					m_selectNoteIcon->m_transform.m_localTransform.m_position = Vector3(
						textPosition.x - textHalfWidth - SELECT_NOTE_GAP - noteMaxHalfWidth,
						textPosition.y,
						0.0f
					);
					m_selectNoteIcon->m_transform.m_localTransform.m_scale = Vector3(noteScale, noteScale, 1.0f);
				}

				// 選択中の文字の下に、文字と同じ横幅の下線を置く
				if (m_selectUnderlineIcon)
				{
					const Vector2& lineSize = m_selectUnderlineIcon->GetSize();
					m_selectUnderlineIcon->m_transform.m_localTransform.m_position = Vector3(
						textPosition.x,
						textPosition.y - textHalfHeight - SELECT_UNDERLINE_GAP - lineSize.y * 0.5f,
						0.0f
					);
					const float lineScaleX = (lineSize.x > 0.0f) ? (textHalfWidth * 2.0f) / lineSize.x : 1.0f;
					m_selectUnderlineIcon->m_transform.m_localTransform.m_scale = Vector3(lineScaleX, 1.0f, 1.0f);
					m_selectUnderlineIcon->m_color = m_eventBaseColors[i];
				}
			}
		}


		bool TitleEventMenu::CaptureEventBaseVisual()
		{
			if (m_isEventBaseVisualCaptured) return true;

			for (auto* text : m_eventIcon)
			{
				if (!text) return false;
			}

			for (size_t i = 0; i < m_eventIcon.size(); ++i)
			{
				m_eventBaseScales[i] = m_eventIcon[i]->GetScale();
				m_eventBaseColors[i] = m_eventIcon[i]->m_color;
			}
			m_isEventBaseVisualCaptured = true;
			return true;
		}


		uint32_t TitleEventMenu::GetSelectKey()const
		{
			return EVENT_ICON_KEYS[static_cast<int>(m_selectIndex)];
		}



		void TitleEventMenu::GetUIParts()
		{
			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("TitleBackGround"));
			if (!m_rogoIcon) m_rogoIcon = GetUI<UIIcon>(Hash32("PentaktRogoIcon"));
			if (!m_selectNoteIcon) m_selectNoteIcon = GetUI<UIIcon>(Hash32("SelectNoteIcon"));
			if (!m_selectUnderlineIcon) m_selectUnderlineIcon = GetUI<UIIcon>(Hash32("SelectUnderlineIcon"));

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
			if (m_selectNoteIcon) m_selectNoteIcon->SetIsDraw(true);
			if (m_selectUnderlineIcon) m_selectUnderlineIcon->SetIsDraw(true);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(true);
			}
		}
	}
}