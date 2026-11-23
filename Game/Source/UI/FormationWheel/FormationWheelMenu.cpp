/**
 * @file FormationWheelMenu.cpp
 * @brief 陣形切り替え(LB/RB)をアイコンで表示するクラス
 */
#include "stdafx.h"
#include "FormationWheelMenu.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"
#include <cmath>

namespace app
{
	namespace ui
	{
		namespace
		{
			/** 陣形種別(EnFormationTypeの値)ごとのJSON要素名 */
			constexpr const char* FORMATION_ICON_NAMES[] = { "SpeedIcon", "DefenseIcon", "GatherIcon" };
			constexpr int FORMATION_NUM = static_cast<int>(actor::EnFormationType::Num);
			static_assert(std::size(FORMATION_ICON_NAMES) == FORMATION_NUM, "アイコンの数が陣形の数と合わない");
			// 3枚なら全アイコンが-1/0/+1に並ぶため、端の1枚を回り込ませる
			static_assert(FORMATION_NUM == 3, "スロットの計算は陣形3つを前提にしている");

			/**
			 * @brief 切り替え中に反対の端へ回り込むアイコンのスロット
			 * @param direction 切り替え方向（+1: 次へ / -1: 前へ）
			 * @param progress  切り替えの進行度（0〜1）
			 * @return 前半は外へ出て消え、後半は反対側から入ってくるスロット
			 */
			constexpr float CalculateWrapSlot(int direction, float progress)
			{
				const float d = static_cast<float>(direction);
				return (progress < 0.5f) ? -d * (1.0f + 2.0f * progress) : d * (3.0f - 2.0f * progress);
			}
			static_assert(CalculateWrapSlot(1, 0.0f) == -1.0f && CalculateWrapSlot(1, 1.0f) == 1.0f, "回り込みの始点と終点がずれている");
			static_assert(CalculateWrapSlot(-1, 0.0f) == 1.0f && CalculateWrapSlot(-1, 1.0f) == -1.0f, "回り込みの始点と終点がずれている");

			/** 見た目チューニングのホットリロード対象JSON */
			constexpr const char* TUNING_JSON_PATH = "Assets/parameter/UI/formationWheel/FormationWheelTuning.json";
			/** チューニングJSONの変更チェック間隔（秒） */
			constexpr float TUNING_RELOAD_INTERVAL = 1.0f;


			float Lerp(float a, float b, float t) { return a + (b - a) * t; }
			float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

			//============================================//
			// 陣形切替カットイン
			// 配置・大きさ・色の基準はFormationWheel.jsonのCutIn* 要素。ここでは動きだけを決める
			//============================================//

			/** カットインのアイコン要素名（EnFormationTypeの並び順） */
			constexpr const char* CUT_IN_ICON_NAMES[] = { "CutInIconSpeed", "CutInIconDefense", "CutInIconGather" };
			/** 陣形名（EnFormationTypeの並び順） */
			constexpr const char* CUT_IN_NAME_TEXTS[] = {
				"はやい",
				"まもり",
				"あつめる",
			};
			/** 副題（EnFormationTypeの並び順）。陣形の効果をひとことで */
			constexpr const char* CUT_IN_SUB_TEXTS[] = {
				"スピード アップ!",
				"クマと シャチを はじく!",
				"子ペンギンが むかえに いく",
			};
			static_assert(std::size(CUT_IN_ICON_NAMES) == FORMATION_NUM, "カットインのアイコンの数が陣形の数と合わない");
			static_assert(std::size(CUT_IN_NAME_TEXTS) == FORMATION_NUM, "陣形名の数が陣形の数と合わない");
			static_assert(std::size(CUT_IN_SUB_TEXTS) == FORMATION_NUM, "副題の数が陣形の数と合わない");

			/** 入り（帯が縦に開き、アイコンと文字が滑り込む）の長さ（秒） */
			constexpr float CUT_IN_IN_TIME = 0.15f;
			/** 見せる時間（アイコンがポップする）の長さ（秒） */
			constexpr float CUT_IN_HOLD_TIME = 0.75f;
			/** 抜け（右へ抜けながら消える）の長さ（秒） */
			constexpr float CUT_IN_OUT_TIME = 0.2f;
			/** HOLDの始まりの時刻（秒） */
			constexpr float CUT_IN_HOLD_BEGIN = CUT_IN_IN_TIME;
			/** OUTの始まりの時刻（秒） */
			constexpr float CUT_IN_OUT_BEGIN = CUT_IN_IN_TIME + CUT_IN_HOLD_TIME;
			/** 全体の長さ（秒）。1.1秒 */
			constexpr float CUT_IN_TOTAL_TIME = CUT_IN_OUT_BEGIN + CUT_IN_OUT_TIME;

			/** 入りでアイコン・文字が滑り込んでくる距離（アイコンは左から、文字は右から） */
			constexpr float CUT_IN_SLIDE_IN_DISTANCE = 60.0f;
			/** 抜けで右へ流れる距離 */
			constexpr float CUT_IN_SLIDE_OUT_DISTANCE = 40.0f;
			/** アイコンのポップの長さ（秒。HOLDの頭から） */
			constexpr float CUT_IN_POP_TIME = 0.25f;
			/** アイコンのポップでの最大拡大率（1.0に足す量） */
			constexpr float CUT_IN_POP_AMOUNT = 0.3f;


			/** 減速する補間（入りに使う） */
			float EaseOutCubic(float t) { const float u = 1.0f - t; return 1.0f - u * u * u; }
			/** 加速する補間（抜けに使う） */
			float EaseInQuad(float t) { return t * t; }
		}


		FormationWheelMenu::FormationWheelMenu()
		{
			LoadTuning();
		}


		void FormationWheelMenu::Update()
		{
			// ゲーム開始時のスライドイン演出（画面外下から所定の位置へ）
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				m_startingAnimLogic.Initialize(
					this,
					{
						"BackgroundIcon",
						"SpeedIcon", "DefenseIcon", "GatherIcon",
						"LBButtonIcon", "RBButtonIcon",
						"LevelText", "FormationNameText", "MemberGaugeBg", "MemberGaugeFill", "MemberGaugeText"
					},
					{}, // 数字UIは使用しないため空のリストを渡す
					Vector3(-400.0f, 0.0f, 0.0f)
				);
			}
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

#if defined(APP_DEBUG)
			ReloadTuningIfChanged(g_gameTime->GetFrameDeltaTime());
#endif
			/** カットインは前フレームの陣形（m_lastFrameType）との比較で発火するため、
			 *  m_lastFrameTypeを更新するUpdateFormationIcons()より先に呼ぶ */
			UpdateFormationCutIn();
			UpdateFormationIcons();
			UpdateLevelDisplay();
			MenuBase::Update();
		}


		void FormationWheelMenu::UpdateFormationCutIn()
		{
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			if (cpm == nullptr)
			{
				m_cutInTimer = -1.0f;
				HideFormationCutIn();
				return;
			}

			/** 陣形が前フレームから変わった瞬間に出す。
			 *  表示中にまた切り替えたら、中身を差し替えてHOLDの頭から見せ直す
			 *  （入りからやり直すと、連打で帯が開閉を繰り返してちらつくため） */
			const actor::EnFormationType currentType = cpm->GetCurrentFormationType();
			if (currentType != m_lastFrameType)
			{
				const bool isShowing = (m_cutInTimer >= 0.0f);
				m_cutInTimer = isShowing ? CUT_IN_HOLD_BEGIN : 0.0f;
				m_cutInType  = currentType;
			}

			if (m_cutInTimer < 0.0f) return;

			m_cutInTimer += g_gameTime->GetFrameDeltaTime();
			if (m_cutInTimer >= CUT_IN_TOTAL_TIME)
			{
				m_cutInTimer = -1.0f;
				HideFormationCutIn();
				return;
			}
			const float t = m_cutInTimer;

			/** 入り: 0→1（減速）。抜け: 0→1（加速） */
			const float inRate  = EaseOutCubic(Clamp01(t / CUT_IN_IN_TIME));
			const float outRate = EaseInQuad(Clamp01((t - CUT_IN_OUT_BEGIN) / CUT_IN_OUT_TIME));
			const float alpha    = inRate * (1.0f - outRate);
			const float slideIn  = CUT_IN_SLIDE_IN_DISTANCE * (1.0f - inRate);
			const float slideOut = CUT_IN_SLIDE_OUT_DISTANCE * outRate;
			/** HOLDの頭でアイコンを一度だけ膨らませて戻す */
			const float popRate = Clamp01((t - CUT_IN_HOLD_BEGIN) / CUT_IN_POP_TIME);
			const float pop     = 1.0f + CUT_IN_POP_AMOUNT * sinf(popRate * Math::PI);

			const int typeIndex = static_cast<int>(m_cutInType);
			const Vector4& formationColor = actor::ChildPenguinManager::GetFormationColor(m_cutInType);

			/** 黒帯: 中心から縦に開き、抜けではその場で消える */
			if (auto* band = GetUI<UIIcon>(Hash32("CutInBand")))
			{
				band->m_isDraw = true;
				band->m_transform.m_localTransform.m_position = m_cutInBandPos;
				band->m_transform.m_localTransform.m_scale =
					Vector3(m_cutInBandScale.x, m_cutInBandScale.y * inRate, m_cutInBandScale.z);
				band->m_color.w = m_cutInBandAlpha * (1.0f - outRate);
			}

			/** 陣形色の線: 帯の縁について開き、抜けでは右へ流れる */
			if (auto* accent = GetUI<UIIcon>(Hash32("CutInAccent")))
			{
				accent->m_isDraw = true;
				Vector3 pos = m_cutInAccentPos;
				pos.x += slideOut;
				pos.y = m_cutInBandPos.y + (m_cutInAccentPos.y - m_cutInBandPos.y) * inRate;
				accent->m_transform.m_localTransform.m_position = pos;
				accent->m_color = Vector4(formationColor.x, formationColor.y, formationColor.z, alpha);
			}

			/** 陣形アイコン: 左から滑り込み、ポップして、右へ抜ける。表示するのは今の陣形の1枚だけ */
			for (int i = 0; i < FORMATION_NUM; i++)
			{
				auto* icon = GetUI<UIIcon>(Hash32(CUT_IN_ICON_NAMES[i]));
				if (icon == nullptr) continue;

				icon->m_isDraw = (i == typeIndex);
				if (!icon->m_isDraw) continue;

				const Vector3& basePos   = m_cutInIconPos[i];
				const Vector3& baseScale = m_cutInIconScale[i];
				icon->m_transform.m_localTransform.m_position = Vector3(basePos.x - slideIn + slideOut, basePos.y, basePos.z);
				icon->m_transform.m_localTransform.m_scale = Vector3(baseScale.x * pop, baseScale.y * pop, baseScale.z);
				icon->m_color.w = alpha;
			}

			/** 陣形名（陣形色）と副題: 右から滑り込み、右へ抜ける */
			if (auto* name = GetUI<UIText>(Hash32("CutInNameText")))
			{
				name->SetText(CUT_IN_NAME_TEXTS[typeIndex]);
				name->m_isDraw = true;
				name->m_transform.m_localTransform.m_position =
					Vector3(m_cutInNamePos.x + slideIn + slideOut, m_cutInNamePos.y, m_cutInNamePos.z);
				name->m_color = Vector4(formationColor.x, formationColor.y, formationColor.z, alpha);
			}
			if (auto* sub = GetUI<UIText>(Hash32("CutInSubText")))
			{
				sub->SetText(CUT_IN_SUB_TEXTS[typeIndex]);
				sub->m_isDraw = true;
				sub->m_transform.m_localTransform.m_position =
					Vector3(m_cutInSubPos.x + slideIn + slideOut, m_cutInSubPos.y, m_cutInSubPos.z);
				sub->m_color.w = alpha;
			}
		}


		void FormationWheelMenu::HideFormationCutIn()
		{
			if (auto* band = GetUI<UIIcon>(Hash32("CutInBand")))       band->m_isDraw = false;
			if (auto* accent = GetUI<UIIcon>(Hash32("CutInAccent")))   accent->m_isDraw = false;
			for (const char* iconName : CUT_IN_ICON_NAMES)
			{
				if (auto* icon = GetUI<UIIcon>(Hash32(iconName)))      icon->m_isDraw = false;
			}
			if (auto* name = GetUI<UIText>(Hash32("CutInNameText")))   name->m_isDraw = false;
			if (auto* sub = GetUI<UIText>(Hash32("CutInSubText")))     sub->m_isDraw = false;
		}


		void FormationWheelMenu::InitializeLogic()
		{
			for (const char* name : FORMATION_ICON_NAMES)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_isDraw = true;
				}
			}

			/** カットインのJSON上の配置を演出の基準として取り込む（ホットリロードのたびに呼ばれる） */
			if (auto* band = GetUI<UIIcon>(Hash32("CutInBand")))
			{
				m_cutInBandPos   = band->m_transform.m_localTransform.m_position;
				m_cutInBandScale = band->m_transform.m_localTransform.m_scale;
				m_cutInBandAlpha = band->m_color.w;
			}
			if (auto* accent = GetUI<UIIcon>(Hash32("CutInAccent")))
			{
				m_cutInAccentPos = accent->m_transform.m_localTransform.m_position;
			}
			for (int i = 0; i < FORMATION_NUM; i++)
			{
				m_cutInIconPos[i]   = Vector3::Zero;
				m_cutInIconScale[i] = Vector3::One;
				if (auto* icon = GetUI<UIIcon>(Hash32(CUT_IN_ICON_NAMES[i])))
				{
					m_cutInIconPos[i]   = icon->m_transform.m_localTransform.m_position;
					m_cutInIconScale[i] = icon->m_transform.m_localTransform.m_scale;
				}
			}
			if (auto* name = GetUI<UIText>(Hash32("CutInNameText")))
			{
				m_cutInNamePos = name->m_transform.m_localTransform.m_position;
			}
			if (auto* sub = GetUI<UIText>(Hash32("CutInSubText")))
			{
				m_cutInSubPos = sub->m_transform.m_localTransform.m_position;
			}

			/** カットインは初期状態では非表示 */
			m_cutInTimer = -1.0f;
			HideFormationCutIn();
		}


		float FormationWheelMenu::CalculateSlotX(float slot) const
		{
			return m_centerX + slot * m_slotSpacing;
		}


		float FormationWheelMenu::CalculateSlotSize(float slot) const
		{
			const float t = Clamp01(std::fabs(slot));
			return Lerp(m_currentSize, m_sideSize, t);
		}


		float FormationWheelMenu::CalculateSlotAlpha(float slot) const
		{
			const float absSlot = std::fabs(slot);
			if (absSlot <= 1.0f) return Lerp(1.0f, m_sideAlpha, absSlot);
			return Lerp(m_sideAlpha, 0.0f, Clamp01(absSlot - 1.0f));
		}


		void FormationWheelMenu::BeginTransition(actor::EnFormationType oldType, actor::EnFormationType newType)
		{
			const int oldIndex = static_cast<int>(oldType);
			const int newIndex = static_cast<int>(newType);
			const int diff = (newIndex - oldIndex + FORMATION_NUM) % FORMATION_NUM;
			// 1: 次へ(RB) / -1: 前へ(LB)
			m_direction = (diff == 1) ? 1 : -1;

			for (int i = 0; i < FORMATION_NUM; i++)
			{
				const int diffFromOld = (i - oldIndex + FORMATION_NUM) % FORMATION_NUM;
				if (diffFromOld == 0)      m_fromSlot[i] = 0;
				else if (diffFromOld == 1) m_fromSlot[i] = 1;
				else                       m_fromSlot[i] = -1;
			}
		}


		void FormationWheelMenu::UpdateFormationIcons()
		{
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			if (cpm == nullptr) return;

			const actor::EnFormationType currentType = cpm->GetCurrentFormationType();
			const bool isSwitching = cpm->IsSwitchingFormation();

			// 切り替え開始エッジを検知し、遷移前のスロット配置を確定する
			if (isSwitching && !m_wasSwitching)
			{
				BeginTransition(m_lastFrameType, currentType);
			}

			const int currentIndex = static_cast<int>(currentType);

			for (int i = 0; i < FORMATION_NUM; i++)
			{
				float slot;
				if (isSwitching)
				{
					const float progress = cpm->GetFormationSwitchProgress();
					slot = (m_fromSlot[i] == -m_direction)
						? CalculateWrapSlot(m_direction, progress)
						: static_cast<float>(m_fromSlot[i]) - static_cast<float>(m_direction) * progress;
				}
				else
				{
					const int diffFromCurrent = (i - currentIndex + FORMATION_NUM) % FORMATION_NUM;
					if (diffFromCurrent == 0)      slot = 0.0f;
					else if (diffFromCurrent == 1) slot = 1.0f;
					else                           slot = -1.0f;
				}

				if (auto* ui = GetUI<UIIcon>(Hash32(FORMATION_ICON_NAMES[i])))
				{
					const float size = CalculateSlotSize(slot);
					const float scaleFactor = size / m_currentSize;
					ui->m_transform.m_localTransform.m_position = Vector3(CalculateSlotX(slot), m_rowY, 0.0f);
					ui->m_transform.m_localTransform.m_scale = Vector3(scaleFactor, scaleFactor, scaleFactor);
					// アイコンは陣形の色（カットインと同じ）で塗る
					const Vector4& formationColor = actor::ChildPenguinManager::GetFormationColor(static_cast<actor::EnFormationType>(i));
					ui->m_color = Vector4(formationColor.x, formationColor.y, formationColor.z, CalculateSlotAlpha(slot));
				}
			}

			m_wasSwitching = isSwitching;
			m_lastFrameType = currentType;
		}


		void FormationWheelMenu::UpdateLevelDisplay()
		{
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			if (cpm == nullptr) return;

			// レベルは内部では0始まりだが、UIでは1始まりの表示にする
			const int level = cpm->GetFormationLevel() + 1;
			const int progress = cpm->GetFormationLevelProgress();
			const int required = cpm->GetFormationNextLevelRequirement();

			// 陣形名はカットインと同じ文字と色で出す
			if (auto* nameText = GetUI<UIText>(Hash32("FormationNameText")))
			{
				const actor::EnFormationType type = cpm->GetCurrentFormationType();
				const Vector4& formationColor = actor::ChildPenguinManager::GetFormationColor(type);
				nameText->SetText(CUT_IN_NAME_TEXTS[static_cast<int>(type)]);
				nameText->m_color = Vector4(formationColor.x, formationColor.y, formationColor.z, nameText->m_color.w);
			}

			if (auto* levelText = GetUI<UIText>(Hash32("LevelText")))
			{
				levelText->SetText("Lv" + std::to_string(level));
			}

			if (auto* gauge = GetUI<UIGauge>(Hash32("MemberGaugeFill")))
			{
				const float fillRate = (required > 0) ? static_cast<float>(progress) / static_cast<float>(required) : 0.0f;
				gauge->m_transform.m_localTransform.m_scale.x = fillRate;
			}

			if (auto* memberText = GetUI<UIText>(Hash32("MemberGaugeText")))
			{
				memberText->SetText(std::to_string(progress) + "/" + std::to_string(required));
			}
		}


		void FormationWheelMenu::LoadTuning()
		{
			nlohmann::json j;
			if (util::JsonConverter::IsLoadJsonFile(j, TUNING_JSON_PATH))
			{
				m_centerX     = util::JsonConverter::ToFloat(j, "centerX", m_centerX);
				m_rowY        = util::JsonConverter::ToFloat(j, "rowY", m_rowY);
				m_slotSpacing = util::JsonConverter::ToFloat(j, "slotSpacing", m_slotSpacing);
				m_currentSize = util::JsonConverter::ToFloat(j, "currentSize", m_currentSize);
				m_sideSize    = util::JsonConverter::ToFloat(j, "sideSize", m_sideSize);
				m_sideAlpha   = util::JsonConverter::ToFloat(j, "sideAlpha", m_sideAlpha);
			}
#if defined(APP_DEBUG)
			m_tuningLastWriteTime = util::JsonConverter::GetFileLastWriteTime(TUNING_JSON_PATH);
#endif
		}


		void FormationWheelMenu::ReloadTuningIfChanged(float dt)
		{
#if defined(APP_DEBUG)
			m_tuningReloadTimer += dt;
			if (m_tuningReloadTimer < TUNING_RELOAD_INTERVAL) return;
			m_tuningReloadTimer = 0.0f;

			if (!util::JsonConverter::CheckFileModified(TUNING_JSON_PATH, m_tuningLastWriteTime)) return;
			LoadTuning();
#endif
		}
	}
}
