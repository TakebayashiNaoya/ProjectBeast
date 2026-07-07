/**
 * @file FeverIconMenu.cpp
 * @brief フィーバータイム開始時に「FEVER」の文字をジャンプで登場・退場させる演出
 * @author 竹林
 */
#include "stdafx.h"
#include "FeverIconMenu.h"
#include "Source/Manager/FeverTimeManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 「FEVER」を構成する文字パーツのキー名（F, E1, V, E2, R の順） */
			constexpr const char* LETTER_KEY_NAMES[] =
			{
				"FeverLetterF",
				"FeverLetterE1",
				"FeverLetterV",
				"FeverLetterE2",
				"FeverLetterR",
			};

			/** 文字ごとのジャンプ開始をずらす時間（波打つように見せるため） */
			constexpr float JUMP_STAGGER_DELAY = 0.08f;
			/** 1文字がジャンプ（登場・退場）にかける時間 */
			constexpr float JUMP_DURATION = 0.4f;
			/** ジャンプの頂点の高さ（着地座標からの相対オフセット） */
			constexpr float JUMP_HEIGHT = 120.0f;
			/** 登場時の開始位置・退場時の落下先の、着地座標からのYオフセット */
			constexpr float JUMP_APPROACH_OFFSET = 80.0f;
			/** 退場時、ジャンプ開始から何割経過した時点で透明化を始めるか（頂点を過ぎて落下し始めた頃に消え始める） */
			constexpr float EXIT_ALPHA_DELAY_RATIO = 0.5f;

			/** フィーバーボイスの音量倍率（通常SEの既定倍率1.0より大きくして目立たせる） */
			constexpr float FEVER_VOICE_VOLUME = 10.0f;

			/** 保持時間などの設定を含むJSONのパス */
			const char* FEVER_ICON_PARAMETER_JSON_PATH = "Assets/parameter/UI/fever/FeverIconParameter.json";
		}


		FeverIconMenu::FeverIconMenu()
		{
			nlohmann::json json;
			if (util::JsonConverter::IsLoadJsonFile(json, FEVER_ICON_PARAMETER_JSON_PATH))
			{
				m_holdDuration = util::JsonConverter::ToFloat(json, "holdDuration", m_holdDuration);
			}
		}


		void FeverIconMenu::Update()
		{
			const bool isFeverActive = FeverTimeManager::GetInstance()->IsActive();

			/** フィーバー開始の瞬間（false→true）にのみ登場演出を開始する */
			if (isFeverActive && !m_wasFeverActive)
			{
				StartEntering();
			}
			m_wasFeverActive = isFeverActive;

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			switch (m_state)
			{
			case EnState::Entering:
				m_stateTimer += deltaTime;
				UpdateLetters(deltaTime);
				if (IsLastLetterFinished())
				{
					m_state = EnState::Holding;
					m_stateTimer = 0.0f;
				}
				break;

			case EnState::Holding:
				m_stateTimer += deltaTime;
				if (m_stateTimer >= m_holdDuration)
				{
					StartExiting();
				}
				break;

			case EnState::Exiting:
				m_stateTimer += deltaTime;
				UpdateLetters(deltaTime);
				if (IsLastLetterFinished())
				{
					m_state = EnState::Idle;
					HideAllLetters();
				}
				break;

			case EnState::Idle:
			default:
				break;
			}

			MenuBase::Update();
		}


		void FeverIconMenu::InitializeLogic()
		{
			m_wasFeverActive = false;
			m_state = EnState::Idle;
			m_stateTimer = 0.0f;

			for (int i = 0; i < LETTER_NUM; i++)
			{
				auto& letter = m_letters[i];
				letter.icon = GetUI<UIIcon>(Hash32(LETTER_KEY_NAMES[i]));
				letter.hasMoveStarted = false;
				letter.hasAlphaStarted = false;

				if (letter.icon)
				{
					letter.finalPos = letter.icon->m_transform.m_localTransform.m_position;
					letter.icon->m_isDraw = false;
					letter.icon->m_color.w = 0.0f;
				}
			}
		}


		void FeverIconMenu::StartEntering()
		{
			m_state = EnState::Entering;
			m_stateTimer = 0.0f;

			/** PlayVoiceは音量倍率を指定できないため、PlaySEで個別に音量を上げて再生する */
			SoundManager::Get().PlaySE(enSoundKind_fever, FEVER_VOICE_VOLUME);

			for (auto& letter : m_letters)
			{
				if (!letter.icon) continue;

				letter.icon->m_isDraw = true;
				letter.icon->m_color.w = 0.0f;
				letter.hasMoveStarted = false;
				letter.hasAlphaStarted = false;
				letter.alphaDelay = 0.0f;

				/** 着地座標の少し下から、頂点を着地座標より高い位置に置いて弧を描かせる */
				Vector3 startPos = letter.finalPos;
				startPos.y -= JUMP_APPROACH_OFFSET;
				Vector3 control = letter.finalPos;
				control.y += JUMP_HEIGHT;

				letter.moveCurve.Initialize(startPos, control, letter.finalPos, JUMP_DURATION);
				/** 登場時はジャンプと同時に不透明化していく */
				letter.alphaCurve.Initialize(0.0f, 1.0f, JUMP_DURATION, util::EasingType::Linear, util::LoopMode::Once);
			}
		}


		void FeverIconMenu::StartExiting()
		{
			m_state = EnState::Exiting;
			m_stateTimer = 0.0f;

			for (auto& letter : m_letters)
			{
				if (!letter.icon) continue;

				letter.hasMoveStarted = false;
				letter.hasAlphaStarted = false;

				/** 着地座標から頂点を経て、着地座標より下（画面外）まで落としながら消す */
				Vector3 control = letter.finalPos;
				control.y += JUMP_HEIGHT;
				Vector3 endPos = letter.finalPos;
				endPos.y -= JUMP_APPROACH_OFFSET;

				letter.moveCurve.Initialize(letter.finalPos, control, endPos, JUMP_DURATION);

				/** 退場時は頂点を過ぎて落下し始めてから透明化する（移動終了と同時に消え終わるよう長さを調整） */
				letter.alphaDelay = JUMP_DURATION * EXIT_ALPHA_DELAY_RATIO;
				const float alphaDuration = JUMP_DURATION - letter.alphaDelay;
				letter.alphaCurve.Initialize(1.0f, 0.0f, alphaDuration, util::EasingType::Linear, util::LoopMode::Once);
			}
		}


		void FeverIconMenu::UpdateLetters(float deltaTime)
		{
			for (int i = 0; i < LETTER_NUM; i++)
			{
				auto& letter = m_letters[i];
				if (!letter.icon) continue;

				const float letterStartTime = i * JUMP_STAGGER_DELAY;

				/** 自分の番（ずらし時間）が来るまでは移動もアルファもまだ動かさない */
				if (!letter.hasMoveStarted)
				{
					if (m_stateTimer < letterStartTime) continue;

					letter.hasMoveStarted = true;
					letter.moveCurve.Play();
				}

				letter.moveCurve.Update(deltaTime);
				letter.icon->m_transform.m_localTransform.m_position = letter.moveCurve.GetCurrentValue();

				/** アルファは移動開始からさらにalphaDelay分待ってから再生を始める */
				if (!letter.hasAlphaStarted)
				{
					if (m_stateTimer < letterStartTime + letter.alphaDelay) continue;

					letter.hasAlphaStarted = true;
					letter.alphaCurve.Play();
				}

				letter.alphaCurve.Update(deltaTime);
				letter.icon->m_color.w = letter.alphaCurve.GetCurrentValue();
			}
		}


		bool FeverIconMenu::IsLastLetterFinished() const
		{
			/** F→E1→V→E2→Rの順で開始するため、最後に開始するR（末尾）の完了だけ見れば全体の完了と等価 */
			const auto& last = m_letters[LETTER_NUM - 1];
			return last.hasMoveStarted && !last.moveCurve.IsPlaying();
		}


		void FeverIconMenu::HideAllLetters()
		{
			for (auto& letter : m_letters)
			{
				if (letter.icon) letter.icon->m_isDraw = false;
			}
		}
	}
}
