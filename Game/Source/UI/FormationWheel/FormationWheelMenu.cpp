/**
 * @file FormationWheelMenu.cpp
 * @brief 陣形切り替え(LB/RB)とウルト発動可否(LT/RT)をアイコンで表示するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationWheelMenu.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Util/CRC32.h"

namespace app
{
	namespace ui
	{
		namespace
		{
			/** 陣形種別ごとのアイコン名の接尾辞（JSON側の要素名と対応） */
			constexpr const char* kFormationSuffixes[] = { "Circle", "Triangle", "Cluster", "Scatter" };
			constexpr int kFormationNum = static_cast<int>(actor::EnFormationType::Num);
		}


		FormationWheelMenu::FormationWheelMenu()
		{}


		void FormationWheelMenu::Update()
		{
			UpdateFormationIcons();
			UpdateUltIconColor();
			MenuBase::Update();
		}


		void FormationWheelMenu::InitializeLogic()
		{
			// 陣形アイコンは毎フレームUpdateFormationIconsで表示状態が確定するため、
			// 初期化時点ではひとまず全て非表示にしておく
			const char* slotPrefixes[] = { "Current", "Prev", "Next" };
			for (const char* prefix : slotPrefixes)
			{
				for (const char* suffix : kFormationSuffixes)
				{
					std::string name = std::string(prefix) + suffix + "Icon";
					if (auto* ui = GetUI<UIIcon>(Hash32(name.c_str())))
					{
						ui->m_isDraw = false;
					}
				}
			}
		}


		void FormationWheelMenu::UpdateFormationIcons()
		{
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			if (cpm == nullptr) return;

			const int current = static_cast<int>(cpm->GetCurrentFormationType());
			const int prev = (current + kFormationNum - 1) % kFormationNum;
			const int next = (current + 1) % kFormationNum;

			auto updateSlot = [&](const char* prefix, int activeType)
				{
					for (int i = 0; i < kFormationNum; i++)
					{
						std::string name = std::string(prefix) + kFormationSuffixes[i] + "Icon";
						if (auto* ui = GetUI<UIIcon>(Hash32(name.c_str())))
						{
							ui->m_isDraw = (i == activeType);
						}
					}
				};

			updateSlot("Current", current);
			updateSlot("Prev", prev);
			updateSlot("Next", next);
		}


		void FormationWheelMenu::UpdateUltIconColor()
		{
			const Vector4 normalColor(1.0f, 1.0f, 1.0f, 1.0f);   // 通常（白）
			const Vector4 grayColor(0.4f, 0.4f, 0.4f, 1.0f);     // グレーアウト

			auto* cpm = actor::ChildPenguinManager::GetInstance();
			const bool canActivate = (cpm != nullptr) && cpm->CanActivateUlt();
			const Vector4& color = canActivate ? normalColor : grayColor;

			const char* ultIconNames[] = { "LTButtonIcon", "RTButtonIcon" };
			for (const char* name : ultIconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_color = color;
				}
			}
		}
	}
}
