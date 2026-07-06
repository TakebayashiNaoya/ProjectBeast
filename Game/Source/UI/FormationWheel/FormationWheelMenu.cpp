/**
 * @file FormationWheelMenu.cpp
 * @brief 陣形切り替え(LB/RB)とウルト発動可否(LT/RT)をアイコンで表示するクラス
 * @author 竹林
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
			constexpr const char* FORMATION_ICON_NAMES[] = { "CircleIcon", "TriangleIcon", "ClusterIcon", "ScatterIcon" };
			constexpr int FORMATION_NUM = static_cast<int>(actor::EnFormationType::Num);

			/** 見た目チューニングのホットリロード対象JSON */
			constexpr const char* TUNING_JSON_PATH = "Assets/parameter/UI/formationWheel/FormationWheelTuning.json";
			/** チューニングJSONの変更チェック間隔（秒） */
			constexpr float TUNING_RELOAD_INTERVAL = 1.0f;


			float Lerp(float a, float b, float t) { return a + (b - a) * t; }
			float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
		}


		FormationWheelMenu::FormationWheelMenu()
		{
			LoadTuning();
		}


		void FormationWheelMenu::Update()
		{
			// ゲーム開始時のスライドイン演出（画面外右から所定の位置へ）
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				m_startingAnimLogic.Initialize(
					this,
					{
						"BackgroundIcon",
						"CircleIcon", "TriangleIcon", "ClusterIcon", "ScatterIcon",
						"LBButtonIcon", "RBButtonIcon", "LTButtonIcon", "RTButtonIcon"
					},
					{}, // 数字UIは使用しないため空のリストを渡す
					Vector3(300.0f, 0.0f, 0.0f)
				);
			}
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			const float dt = g_gameTime->GetFrameDeltaTime();
			m_pulseTimer += dt;
#if defined(APP_DEBUG)
			ReloadTuningIfChanged(dt);
#endif
			UpdateFormationIcons();
			UpdateUltIconColor();
			MenuBase::Update();
		}


		void FormationWheelMenu::InitializeLogic()
		{
			for (const char* name : FORMATION_ICON_NAMES)
			{
				if (auto* ui = GetUI<UILinearFillGauge>(Hash32(name)))
				{
					ui->m_isDraw = true;
				}
			}
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


		float FormationWheelMenu::CalculatePulseScale(bool isPulsing) const
		{
			if (!isPulsing) return 1.0f;
			return 1.0f + m_pulseAmplitude * sinf(m_pulseTimer * m_pulseSpeed);
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
				if (diffFromOld == 0)                      m_fromSlot[i] = 0;
				else if (diffFromOld == 1)                 m_fromSlot[i] = 1;
				else if (diffFromOld == FORMATION_NUM - 1) m_fromSlot[i] = -1;
				else                                       m_fromSlot[i] = (m_direction > 0) ? 2 : -2;
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

			// ウルトの充填率：発動中は残り時間割合(上から白くなっていく)、それ以外はチャージ進行度(下から黄色くなる)
			const bool isUltActive = cpm->IsUltActive();
			const float ultFillAmount = isUltActive
				? cpm->GetUltActiveRemainingRate()
				: (1.0f - cpm->GetUltCooldownRate());

			// パルス演出・ウルト充填演出は「現在選択中の陣形」のアイコンにのみ反映する
			const bool pulseFormation = cpm->CanActivateUlt() || isUltActive;
			const float pulseScale = CalculatePulseScale(pulseFormation);

			const int currentIndex = static_cast<int>(currentType);

			for (int i = 0; i < FORMATION_NUM; i++)
			{
				float slot;
				if (isSwitching)
				{
					const float progress = cpm->GetFormationSwitchProgress();
					slot = static_cast<float>(m_fromSlot[i]) - static_cast<float>(m_direction) * progress;
				}
				else
				{
					const int diffFromCurrent = (i - currentIndex + FORMATION_NUM) % FORMATION_NUM;
					if (diffFromCurrent == 0)                      slot = 0.0f;
					else if (diffFromCurrent == 1)                 slot = 1.0f;
					else if (diffFromCurrent == FORMATION_NUM - 1) slot = -1.0f;
					else                                           slot = 2.0f; // 非表示側（見えないので向きは任意）
				}

				if (auto* ui = GetUI<UILinearFillGauge>(Hash32(FORMATION_ICON_NAMES[i])))
				{
					const bool isCurrent = (i == currentIndex);
					const float size = CalculateSlotSize(slot);
					const float scaleFactor = (size / m_currentSize) * (isCurrent ? pulseScale : 1.0f);
					ui->m_transform.m_localTransform.m_position = Vector3(CalculateSlotX(slot), m_rowY, 0.0f);
					ui->m_transform.m_localTransform.m_scale = Vector3(scaleFactor, scaleFactor, scaleFactor);
					ui->m_color = Vector4(m_iconColor.x / 255.0f, m_iconColor.y / 255.0f, m_iconColor.z / 255.0f, CalculateSlotAlpha(slot));
					ui->SetFillAmount(isCurrent ? ultFillAmount : 0.0f);
				}
			}

			m_wasSwitching = isSwitching;
			m_lastFrameType = currentType;
		}


		void FormationWheelMenu::UpdateUltIconColor()
		{
			const Vector4 readyColor(1.0f, 1.0f, 0.0f, 1.0f);    // 黄色（発動可能）
			const Vector4 grayColor(0.4f, 0.4f, 0.4f, 1.0f);     // グレーアウト（発動不可）

			auto* cpm = actor::ChildPenguinManager::GetInstance();
			const bool canActivate = (cpm != nullptr) && cpm->CanActivateUlt();
			const Vector4& color = canActivate ? readyColor : grayColor;

			// 発動可能な間だけボタンをドクンドクン拡縮させる（発動中は等倍に戻す）
			const float pulseScale = CalculatePulseScale(canActivate);

			const char* ultIconNames[] = { "LTButtonIcon", "RTButtonIcon" };
			for (const char* name : ultIconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_color = color;
					ui->m_transform.m_localTransform.m_scale = Vector3(pulseScale, pulseScale, pulseScale);
				}
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
				m_iconColor   = util::JsonConverter::ToVector3(j, "iconColor", false, m_iconColor);
				m_pulseAmplitude = util::JsonConverter::ToFloat(j, "pulseAmplitude", m_pulseAmplitude);
				m_pulseSpeed     = util::JsonConverter::ToFloat(j, "pulseSpeed", m_pulseSpeed);
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
