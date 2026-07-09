/**
 * @file TutorialController.cpp
 * @brief チュートリアルステージのトリガー・ウィンドウ管理
 * @author 竹林
 */
#include "stdafx.h"
#include "SceneManager.h"
#include "TutorialController.h"

#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"

#include "Source/Achivement/AchievementManager.h"


namespace app
{
	// ---------- 静的メンバー定義 ----------

	static constexpr const char* TUTORIAL_STEP_NAMES[] = {
		"PenguinSerious", "PenguinClingy", "PenguinNaughty", "PenguinClumsy",
		"PenguinCaring",  "Bear",          "Igloo",           "Whirlpool"
	};

	const char* const TutorialController::WINDOW_JSON_PATHS[TARGET_COUNT] =
	{
		"Assets/parameter/Tutorial/TutorialWindow_PenguinSerious.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinClingy.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinNaughty.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinClumsy.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinCaring.json",
		"Assets/parameter/Tutorial/TutorialWindow_Bear.json",
		"Assets/parameter/Tutorial/TutorialWindow_Igloo.json",
		"Assets/parameter/Tutorial/TutorialWindow_Whirlpool.json",
	};


	// ---------- 公開インタフェース ----------

	void TutorialController::Initialize(actor::DaddyPenguin* daddy)
	{
		m_daddy = daddy;

		for (int i = 0; i < TARGET_COUNT; i++)
			m_windowLayouts[i].Initialize<ui::TutorialWindowMenu>(WINDOW_JSON_PATHS[i]);
	}


	void TutorialController::Update()
	{
		if (!m_daddy) return;

		const Vector3 daddyPos = m_daddy->GetTransform().m_position;
		const float triggerRadSq = TRIGGER_RADIUS * TRIGGER_RADIUS;

		// 近接トリガー検知
		for (int i = 0; i < PROXIMITY_TARGET_COUNT; i++)
		{
			const auto target = PROXIMITY_TARGETS[i];
			const int  idx = static_cast<int>(target);
			if (m_triggered[idx]) continue;

			Vector3 targetPos;
			if (!GetNearestTargetPosition(target, targetPos)) continue;

			const Vector3 diff = daddyPos - targetPos;
			if (diff.LengthSq() < triggerRadSq)
			{
				m_triggered[idx] = true;
				m_queue.push(target);
			}
		}

		// 表示中ウィンドウの更新（ポーズさせず毎フレーム動かす）
		if (m_isWindowOpen)
		{
			m_windowLayouts[m_currentTargetIdx].Update();

			auto* menu = m_windowLayouts[m_currentTargetIdx].GetMenu<ui::TutorialWindowMenu>();

			if (m_windowDisplayTimer < TUTORIAL_WINDOW_DISPLAY_TIME)
			{
				m_windowDisplayTimer += g_gameTime->GetFrameDeltaTime();
				if (m_windowDisplayTimer >= TUTORIAL_WINDOW_DISPLAY_TIME && menu)
					menu->RequestClose();
			}

			if (menu && menu->IsClosed())
				FinishCurrentWindow();
		}

		if (!m_isWindowOpen && !m_queue.empty())
			TryOpenNextWindow();
	}


	void TutorialController::Render(RenderContext& rc)
	{
		if (m_isWindowOpen)
			m_windowLayouts[m_currentTargetIdx].Render(rc);
	}


	bool TutorialController::PauseUpdate()
	{
		return false;
	}


	bool TutorialController::PauseRender(RenderContext& rc)
	{
		return false;
	}


	// ---------- 非公開実装 ----------

	bool TutorialController::GetNearestTargetPosition(EnTutorialTarget type, Vector3& outPos) const
	{
		// ペンギン5種
		if (type >= EnTutorialTarget::PenguinSerious && type <= EnTutorialTarget::PenguinCaring)
		{
			const auto penguinType = static_cast<actor::EnChildPenguinType>(
				static_cast<int>(type) - static_cast<int>(EnTutorialTarget::PenguinSerious));

			const Vector3 from = m_daddy->GetTransform().m_position;
			float minDistSq = FLT_MAX;
			bool  found = false;

			for (auto* p : actor::ChildPenguinManager::GetInstance()->GetChildPenguin())
			{
				if (!p || p->GetChildPenguinType() != penguinType) continue;
				const float dSq = (from - p->GetTransform().m_position).LengthSq();
				if (dSq < minDistSq) { minDistSq = dSq; outPos = p->GetTransform().m_position; found = true; }
			}
			return found;
		}

		switch (type)
		{
		case EnTutorialTarget::Bear:
			return actor::EnemyManager::GetInstance()->GetNearestEnemyPosition(
				m_daddy->GetTransform().m_position, outPos);

		case EnTutorialTarget::Igloo:
		{
			const Vector3 pos = actor::StageSystem::GetInstance()->GetNearestIglooPosition(
				m_daddy->GetTransform().m_position);
			if (pos.LengthSq() < 0.001f) return false;
			outPos = pos;
			return true;
		}

		case EnTutorialTarget::Whirlpool:
		{
			const Vector3 from = m_daddy->GetTransform().m_position;
			float minDistSq = FLT_MAX;
			bool  found = false;

			if (auto* wm = nature::WhirlpoolManager::GetInstance())
			{
				wm->ForEach([&](nature::Whirlpool* w)
					{
						if (!w) return;
						const float dSq = (from - w->GetTransform().m_position).LengthSq();
						if (dSq < minDistSq) { minDistSq = dSq; outPos = w->GetTransform().m_position; found = true; }
					});
				return found;
			}
		}

		default:
			return false;
		}
	}


	void TutorialController::TryOpenNextWindow()
	{
		if (m_queue.empty()) return;

		const auto target = m_queue.front();
		m_queue.pop();
		m_currentTargetIdx = static_cast<int>(target);

		auto* menu = m_windowLayouts[m_currentTargetIdx].GetMenu<ui::TutorialWindowMenu>();
		if (menu)
		{
			menu->Open();
			m_isWindowOpen = true;
			m_windowDisplayTimer = 0.0f;
		}
		else
		{
			// Layout 初期化失敗（JSON 欠損など） — エントリをスキップして完了済みにする
			if (auto* lm = GameLogManager::GetInstance())
				lm->QueueEvent({ {"ev", "tutorial_complete"}, {"step", TUTORIAL_STEP_NAMES[m_currentTargetIdx]} });
			if (auto* am = app::achievement::AchievementManager::GetInstance())
			{
				auto* base = am->GetAchievement(Hash32(TUTORIAL_STEP_NAMES[m_currentTargetIdx]));
				if (auto* ev = dynamic_cast<app::achievement::EventAchievement*>(base))
					ev->Unlock();
			}
		}
	}


	void TutorialController::FinishCurrentWindow()
	{
		m_isWindowOpen = false;

		if (auto* lm = GameLogManager::GetInstance())
			lm->QueueEvent({ {"ev", "tutorial_complete"}, {"step", TUTORIAL_STEP_NAMES[m_currentTargetIdx]} });
		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			auto* base = am->GetAchievement(Hash32(TUTORIAL_STEP_NAMES[m_currentTargetIdx]));
			if (auto* ev = dynamic_cast<app::achievement::EventAchievement*>(base))
				ev->Unlock();
		}
	}
}
