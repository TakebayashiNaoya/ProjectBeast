/**
 * @file InGameHintMenu.cpp
 * @brief インゲームの初回操作ヒント（ポップ表示）
 * @author 竹林
 */
#include "stdafx.h"
#include "InGameHintMenu.h"

#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** ポップイン（上からストンと入る）の長さ（秒） */
			constexpr float POP_IN_DURATION = 0.25f;
			/** 表示保持の長さ（秒） */
			constexpr float HOLD_DURATION = 2.5f;
			/** フェードアウトの長さ（秒） */
			constexpr float FADE_OUT_DURATION = 0.3f;

			/** ポップイン開始時の拡大率（1.0へ縮みながら着地する） */
			constexpr float POP_IN_START_SCALE = 1.3f;

			/** 重要ヒント表示時のスローモーション（時間倍率・実時間秒）。見逃し防止 */
			constexpr float HINT_SLOW_MOTION_SCALE = 0.5f;
			constexpr float HINT_SLOW_MOTION_DURATION = 0.5f;

			/** ヒント表示SEの音量倍率 */
			constexpr float HINT_SE_VOLUME = 1.0f;

			/** 1ヒントが使うアイコンの最大数（例: ウルトは LT / RT） */
			constexpr int HINT_ICON_MAX = 3;

			/** ヒントの定義（アイコンのUI名・本文・重要かどうか） */
			/** 帯のJSON上の基準幅。ヒントごとの帯幅はこれに対する横スケールで表現する */
			constexpr float BAND_BASE_WIDTH = 700.0f;

			struct HintDef
			{
				const char* iconNames[HINT_ICON_MAX]; /** 表示するアイコンのUI名（未使用枠はnullptr） */
				const char* text;                     /** 本文（ひらがな短文・左揃え） */
				float       textOffsetX;              /** 本文の左端X（スクリーン座標） */
				float       bandWidth;                /** 帯の幅（文の長さに合わせて余白を作らない） */
				bool        hasSlowMotion;            /** 見逃せないヒントは表示時に軽いスローを添える */
			};

			/** EnHintType と添字を対応させた定義テーブル */
			constexpr HintDef HINT_DEFS[static_cast<uint8_t>(EnHintType::Num)] =
			{
				/** Regroup */
				{ { "HintIconRegroup", nullptr, nullptr },
					"みんなを よびもどそう！", -235.0f, 540.0f, true },
				/** Ult（LT / RT のどちらでも撃てるため両方見せる） */
				{ { "HintIconUltL", "HintIconSlash", "HintIconUlt" },
					"ひっさつわざが うてるよ！", -165.0f, 630.0f, true },
				/** Sneak */
				{ { "HintIconSneak", nullptr, nullptr },
					"そーっと あるこう！", -235.0f, 475.0f, false },
				/** Slide */
				{ { "HintIconSlide", nullptr, nullptr },
					"さかは すべると はやい！", -235.0f, 575.0f, false },
			};

			/** スニークヒント：寝ているシロクマを探す半径。
			 *  近すぎる値にすると、到達する前に足音でクマが起きてしまい一度も発火しない
			 *  （250ではボット実測で最接近344止まり。2026-08-26） */
			constexpr float SNEAK_HINT_RANGE = 600.0f;
			/** スライドヒント：下り坂とみなす符号つき傾斜（sin値。子AIのスライド選択と同じ） */
			constexpr float SLIDE_HINT_SLOPE = 0.06f;

			/** 全ヒント共通パーツのUI名 */
			constexpr const char* BAND_NAME = "HintBand";
			constexpr const char* TEXT_NAME = "HintText";
		}


		InGameHintMenu::InGameHintMenu()
		{}


		void InGameHintMenu::InitializeLogic()
		{
			// Reload後に古い表示が残らないようリセットする（表示済みフラグは維持しない）
			m_showState = EnShowState::Hidden;
			m_currentType = EnHintType::Num;
			m_showTimer = 0.0f;
			m_isShown.fill(false);
			m_queue.clear();

			// 全パーツを非表示にする
			m_iconBasePositions.clear();
			if (auto* band = GetUI<UIIcon>(Hash32(BAND_NAME)))
			{
				band->SetIsDraw(false);
			}
			if (auto* text = GetUI<UIText>(Hash32(TEXT_NAME)))
			{
				text->SetIsDraw(false);
				m_textBasePosition = text->m_transform.m_localTransform.m_position;

				// 本文はアイコンの右から始める左揃え（中央揃えだと長文がアイコンに被る）
				text->SetTextAlign(nsBeastEngine::TextAlign::Left);
				text->m_pivot = Vector2(0.0f, 0.5f);
			}
			for (const auto& def : HINT_DEFS)
			{
				for (const char* iconName : def.iconNames)
				{
					if (iconName == nullptr) continue;
					if (auto* icon = GetUI<UIIcon>(Hash32(iconName)))
					{
						icon->SetIsDraw(false);
						m_iconBasePositions[Hash32(iconName)] = icon->m_transform.m_localTransform.m_position;
					}
				}
			}
		}


		void InGameHintMenu::Update()
		{
			UpdateTriggers();

			// 非表示中にキューが残っていれば次のヒントを出す
			if (m_showState == EnShowState::Hidden && !m_queue.empty())
			{
				const EnHintType next = m_queue.front();
				m_queue.erase(m_queue.begin());
				Show(next);
			}

			UpdateShowAnim();

			MenuBase::Update();
		}


		void InGameHintMenu::UpdateTriggers()
		{
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			if (cpm == nullptr) return;

			// Y（よびもどし）：自分の隊列の子がクマに襲われていて、再集合が使えるとき
			if (cpm->HasBearThreatOnFormation() && cpm->CanCallRegroup())
			{
				Enqueue(EnHintType::Regroup);
			}

			// ウルト：初めて満タンになったとき（初回は半分チャージから始まるため、
			// プレイ開始直後ではなく少し遊んだタイミングで発生する）
			if (cpm->CanActivateUlt())
			{
				Enqueue(EnHintType::Ult);
			}

			// スニーク：寝ているシロクマに近づいたとき（そっと歩けば起こさない）
			auto* em = actor::EnemyManager::GetInstance();
			if (em != nullptr
				&& em->GetNearestSleepingEnemy(cpm->GetDaddyPosition(), SNEAK_HINT_RANGE) != nullptr)
			{
				Enqueue(EnHintType::Sneak);
			}

			// スライド：歩き・走りで下り坂にさしかかったとき（すでに滑っていれば教える必要はない）
			auto* daddy = cpm->GetDaddyPenguin();
			if (daddy != nullptr && daddy->GetStateMachine() != nullptr)
			{
				auto* stateMachine = daddy->GetStateMachine();
				if (!stateMachine->GetIsSlide()
					&& stateMachine->GetSlideSlopeSigned() > SLIDE_HINT_SLOPE)
				{
					Enqueue(EnHintType::Slide);
				}
			}

		}


		void InGameHintMenu::Enqueue(EnHintType type)
		{
			const uint8_t index = static_cast<uint8_t>(type);
			if (m_isShown[index]) return;

			// 積んだ時点で表示済み扱いにして、多重登録を防ぐ
			m_isShown[index] = true;
			m_queue.push_back(type);
		}


		void InGameHintMenu::Show(EnHintType type)
		{
			m_currentType = type;
			m_showState = EnShowState::PopIn;
			m_showTimer = 0.0f;

			const HintDef& def = HINT_DEFS[static_cast<uint8_t>(type)];

			// 帯は画面中央のまま幅だけ文の長さに合わせ、中身（アイコン・本文）を
			// 狭くなった分だけ右へ寄せて帯の中央に収める
			m_bandScaleX = def.bandWidth / BAND_BASE_WIDTH;
			const float contentShiftX = (BAND_BASE_WIDTH - def.bandWidth) * 0.5f;

			if (auto* text = GetUI<UIText>(Hash32(TEXT_NAME)))
			{
				text->SetText(def.text);

				// 本文の左端Xはヒントごとの定義値を使う（アイコンの本数が違うため）
				text->m_transform.m_localTransform.m_position = Vector3(
					def.textOffsetX + contentShiftX, m_textBasePosition.y, m_textBasePosition.z);
			}

			for (const char* iconName : def.iconNames)
			{
				if (iconName == nullptr) continue;
				if (auto* icon = GetUI<UIIcon>(Hash32(iconName)))
				{
					const Vector3& base = m_iconBasePositions[Hash32(iconName)];
					icon->m_transform.m_localTransform.m_position =
						Vector3(base.x + contentShiftX, base.y, base.z);
				}
			}

			SoundManager::Get().PlaySE(enSoundKind_NoticeAchievement, HINT_SE_VOLUME);

			// 見逃せないヒントは軽いスローモーションを添える
			if (def.hasSlowMotion)
			{
				g_gameTime->StartSlowMotion(HINT_SLOW_MOTION_SCALE, HINT_SLOW_MOTION_DURATION);
			}
		}


		void InGameHintMenu::UpdateShowAnim()
		{
			if (m_showState == EnShowState::Hidden) return;

			m_showTimer += g_gameTime->GetFrameDeltaTime();

			switch (m_showState)
			{
			case EnShowState::PopIn:
			{
				const float u = (std::min)(m_showTimer / POP_IN_DURATION, 1.0f);
				const float scale = POP_IN_START_SCALE + (1.0f - POP_IN_START_SCALE) * u;
				ApplyVisual(scale, u);
				if (m_showTimer >= POP_IN_DURATION)
				{
					m_showState = EnShowState::Hold;
					m_showTimer = 0.0f;
				}
				break;
			}

			case EnShowState::Hold:
			{
				ApplyVisual(1.0f, 1.0f);
				if (m_showTimer >= HOLD_DURATION)
				{
					m_showState = EnShowState::FadeOut;
					m_showTimer = 0.0f;
				}
				break;
			}

			case EnShowState::FadeOut:
			{
				const float u = (std::min)(m_showTimer / FADE_OUT_DURATION, 1.0f);
				ApplyVisual(1.0f, 1.0f - u);
				if (m_showTimer >= FADE_OUT_DURATION)
				{
					m_showState = EnShowState::Hidden;
					m_showTimer = 0.0f;
					ApplyVisual(1.0f, 0.0f);
				}
				break;
			}

			default:
				break;
			}
		}


		void InGameHintMenu::ApplyVisual(float scale, float alpha)
		{
			const bool isVisible = (m_showState != EnShowState::Hidden) && (alpha > 0.0f);

			if (auto* band = GetUI<UIIcon>(Hash32(BAND_NAME)))
			{
				band->SetIsDraw(isVisible);
				band->m_transform.m_localTransform.m_scale = Vector3(scale * m_bandScaleX, scale, 1.0f);
				band->m_color.w = 0.55f * alpha;
			}

			if (auto* text = GetUI<UIText>(Hash32(TEXT_NAME)))
			{
				text->SetIsDraw(isVisible);
				text->m_color.w = alpha;
			}

			// アイコンは表示中のヒントのものだけを出す
			for (uint8_t i = 0; i < static_cast<uint8_t>(EnHintType::Num); ++i)
			{
				const bool isCurrent = (m_currentType == static_cast<EnHintType>(i));
				for (const char* iconName : HINT_DEFS[i].iconNames)
				{
					if (iconName == nullptr) continue;
					auto* icon = GetUI<UIIcon>(Hash32(iconName));
					if (icon == nullptr) continue;

					icon->SetIsDraw(isVisible && isCurrent);
					icon->m_transform.m_localTransform.m_scale = Vector3(scale, scale, 1.0f);
					icon->m_color.w = alpha;
				}
			}
		}
	}
}
