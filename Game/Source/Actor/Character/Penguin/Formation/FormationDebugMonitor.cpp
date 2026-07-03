/**
 * @file FormationDebugMonitor.cpp
 * @brief 陣形の状態をImGuiデバッグウィンドウで監視するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationDebugMonitor.h"
#include "Source/Core/DebugWindow.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** DebugWindow に登録するセクション名 */
			constexpr const char* DEBUG_LABEL = u8"陣形";

			/** @brief 陣形種別を表示名に変換する */
			const char* ToFormationName(EnFormationType type)
			{
				switch (type)
				{
				case EnFormationType::Circle:   return u8"円陣（通常）";
				case EnFormationType::Triangle: return u8"三角陣（スピード特化）";
				case EnFormationType::Cluster:  return u8"密集陣（防御特化）";
				case EnFormationType::Scatter:  return u8"散開陣（収集特化）";
				default:                        return u8"不明";
				}
			}
		}


		FormationDebugMonitor::FormationDebugMonitor(const ChildPenguinManager* manager)
			: m_manager(manager)
		{
			app::DebugWindow::Get().Register(DEBUG_LABEL, [this] { Draw(); });
		}


		FormationDebugMonitor::~FormationDebugMonitor()
		{
			app::DebugWindow::Get().Unregister(DEBUG_LABEL);
		}


		void FormationDebugMonitor::Draw() const
		{
			if (m_manager == nullptr) return;

			// ── 陣形共通情報 ──────────────────────
			ImGui::SeparatorText(u8"陣形共通");
			ImGui::Text(u8"陣形種別: %s", ToFormationName(m_manager->GetCurrentFormationType()));
			ImGui::Text(u8"陣形レベル: %d", m_manager->GetFormationLevel());
			ImGui::Text(u8"子ペンギン総数: %d", m_manager->GetChildPenguinNum());
			ImGui::Text(u8"救出済み(隊列)数: %d", m_manager->GetFollowersNum());
			ImGui::Text(u8"うち甘えん坊数: %d", m_manager->GetClingyCount());

			ImGui::Text(u8"救出範囲(入隊判定半径): %.1f", m_manager->GetJoinRadius());
			ImGui::Text(u8"最外半径: %.1f", m_manager->GetOuterRadius());

			// ── 発動中の効果 ──────────────────────
			ImGui::SeparatorText(u8"発動中の効果");
			ImGui::Text(u8"速度倍率(パッシブ×ウルト): %.2fx", m_manager->GetFormationSpeedMultiplier());
			ImGui::Text(u8"渦潮耐性: %s", m_manager->HasWhirlpoolResistance() ? u8"あり" : u8"なし");

			ImGui::Text(u8"ウルト発動中: %s", m_manager->IsUltActive() ? u8"はい" : u8"いいえ");
			ImGui::Text(u8"ウルト発動可能: %s", m_manager->CanActivateUlt() ? u8"はい" : u8"いいえ");
			ImGui::ProgressBar(m_manager->GetUltCooldownRate(), ImVec2(-1.0f, 0.0f));
			ImGui::SameLine();
			ImGui::TextDisabled(u8"クールダウン率");
		}
	}
}
