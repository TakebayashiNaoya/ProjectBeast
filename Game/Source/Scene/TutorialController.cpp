/**
 * @file TutorialController.cpp
 * @brief チュートリアルステージのトリガー・矢印・ウィンドウ管理
 * @author 竹林
 */
#include "stdafx.h"
#include "TutorialController.h"
#include "SceneManager.h"

#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Nature/Whirlpool.h"

#include "Graphics/Camera/CameraSystem.h"


namespace app
{
	// ---------- 静的メンバー定義 ----------

	const EnTutorialTarget TutorialController::ARROW_TARGETS[ARROW_COUNT] =
	{
		EnTutorialTarget::PenguinSerious,
		EnTutorialTarget::PenguinClingy,
		EnTutorialTarget::PenguinNaughty,
		EnTutorialTarget::PenguinClumsy,
		EnTutorialTarget::PenguinCaring,
		EnTutorialTarget::Bear,
		EnTutorialTarget::BearNest,
		EnTutorialTarget::Igloo,
		EnTutorialTarget::Whirlpool,
	};

	const char* const TutorialController::WINDOW_JSON_PATHS[TARGET_COUNT] =
	{
		"Assets/parameter/Tutorial/TutorialWindow_PenguinSerious.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinClingy.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinNaughty.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinClumsy.json",
		"Assets/parameter/Tutorial/TutorialWindow_PenguinCaring.json",
		"Assets/parameter/Tutorial/TutorialWindow_Bear.json",
		"Assets/parameter/Tutorial/TutorialWindow_BearNest.json",
		"Assets/parameter/Tutorial/TutorialWindow_Igloo.json",
		"Assets/parameter/Tutorial/TutorialWindow_Ocean.json",
		"Assets/parameter/Tutorial/TutorialWindow_Whirlpool.json",
	};


	// ---------- 公開インタフェース ----------

	void TutorialController::Initialize(actor::DaddyPenguin* daddy)
	{
		m_daddy = daddy;

		for (int i = 0; i < TARGET_COUNT; i++)
			m_windowLayouts[i].Initialize<ui::TutorialWindowMenu>(WINDOW_JSON_PATHS[i]);

		for (auto& packet : m_arrowPackets)
			packet.Initialize("Assets/parameter/UI/tutorialArrow/TutorialArrow.json");
	}


	void TutorialController::Update()
	{
		if (!m_daddy) return;

		CheckProximityTriggers();

		if (!m_isWindowOpen && !m_queue.empty())
			TryOpenNextWindow();

		UpdateArrows();
	}


	void TutorialController::Render(RenderContext& rc)
	{
		for (auto& packet : m_arrowPackets)
			packet.Render(rc);
	}


	bool TutorialController::PauseUpdate()
	{
		if (!m_isWindowOpen) return false;

		m_windowLayouts[m_currentTargetIdx].Update();

		auto* menu = m_windowLayouts[m_currentTargetIdx].GetMenu<ui::TutorialWindowMenu>();
		if (menu && menu->IsClosedByUser())
		{
			m_completed[m_currentTargetIdx] = true;
			m_isWindowOpen = false;

			if (!m_queue.empty())
			{
				TryOpenNextWindow();
			}
			else
			{
				SceneManager::GetInstance()->SetPause(false);
			}
		}

		return true;
	}


	bool TutorialController::PauseRender(RenderContext& rc)
	{
		if (!m_isWindowOpen) return false;

		m_windowLayouts[m_currentTargetIdx].Render(rc);
		return true;
	}


	// ---------- 非公開実装 ----------

	void TutorialController::CheckProximityTriggers()
	{
		const Vector3 daddyPos = m_daddy->GetTransform().m_position;

		for (int i = 0; i < TARGET_COUNT; i++)
		{
			if (m_triggered[i]) continue;

			const auto target = static_cast<EnTutorialTarget>(i);
			bool triggered = false;

			if (target == EnTutorialTarget::Ocean)
			{
				triggered = m_daddy->GetStateMachine()->IsSwimming();
			}
			else
			{
				Vector3 nearestPos;
				if (GetNearestTargetPosition(target, nearestPos))
				{
					const Vector3 diff = daddyPos - nearestPos;
					triggered = diff.LengthSq() < TRIGGER_RADIUS * TRIGGER_RADIUS;
				}
			}

			if (triggered)
			{
				m_triggered[i] = true;
				m_queue.push(target);
			}
		}
	}


	bool TutorialController::GetNearestTargetPosition(EnTutorialTarget type, Vector3& outPos) const
	{
		// ペンギン5種
		if (type >= EnTutorialTarget::PenguinSerious && type <= EnTutorialTarget::PenguinCaring)
		{
			const auto penguinType = static_cast<actor::EnChildPenguinType>(
				static_cast<int>(type) - static_cast<int>(EnTutorialTarget::PenguinSerious));

			const Vector3  from = m_daddy->GetTransform().m_position;
			float          minDistSq = FLT_MAX;
			bool           found = false;

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
		{
			const auto positions = actor::EnemyManager::GetInstance()->GetPositionList();
			if (positions.empty()) return false;

			const Vector3 from = m_daddy->GetTransform().m_position;
			float         minDistSq = FLT_MAX;

			for (const auto& pos : positions)
			{
				const float dSq = (from - pos).LengthSq();
				if (dSq < minDistSq) { minDistSq = dSq; outPos = pos; }
			}
			return true;
		}

		case EnTutorialTarget::BearNest:
		{
			const Vector3 from = m_daddy->GetTransform().m_position;
			const Vector3 pos = actor::StageSystem::GetInstance()->GetNearestBearNestPosition(from);
			if (pos.LengthSq() < 0.001f) return false;
			outPos = pos;
			return true;
		}

		case EnTutorialTarget::Igloo:
		{
			const Vector3 from = m_daddy->GetTransform().m_position;
			const Vector3 pos = actor::StageSystem::GetInstance()->GetNearestIglooPosition(from);
			if (pos.LengthSq() < 0.001f) return false;
			outPos = pos;
			return true;
		}

		case EnTutorialTarget::Whirlpool:
		{
			const Vector3 from = m_daddy->GetTransform().m_position;
			float         minDistSq = FLT_MAX;
			bool          found = false;

			nature::WhirlpoolManager::GetInstance()->ForEach([&](nature::Whirlpool* w)
				{
					if (!w) return;
					const float dSq = (from - w->GetTransform().m_position).LengthSq();
					if (dSq < minDistSq) { minDistSq = dSq; outPos = w->GetTransform().m_position; found = true; }
				});
			return found;
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
			SceneManager::GetInstance()->SetPause(true);
		}
	}


	void TutorialController::UpdateArrows()
	{
		auto& mainCamera = CameraSystem::Get().GetMainCamera();
		const Vector3 camPos = mainCamera.GetPosition();
		const Vector3 camFwd = mainCamera.GetForward();
		const Frustum& frustum = g_renderingEngine->GetFrustum();

		for (int i = 0; i < ARROW_COUNT; i++)
		{
			const auto target = ARROW_TARGETS[i];
			auto* menu = m_arrowPackets[i].GetMenu();
			if (!menu) continue;

			const int idx = static_cast<int>(target);

			if (m_completed[idx])
			{
				menu->SetVisible(false);
				m_arrowPackets[i].Update();
				continue;
			}

			Vector3 targetPos;
			if (!GetNearestTargetPosition(target, targetPos))
			{
				menu->SetVisible(false);
				m_arrowPackets[i].Update();
				continue;
			}

			// ワールド座標 → スクリーン座標
			Vector2 screenPos;
			mainCamera.CalcScreenPositionFromWorldPosition(screenPos, targetPos);

			// カメラ背後の符号反転補正
			const Vector3 toTarget = targetPos - camPos;
			if (toTarget.Dot(camFwd) < 0.0f)
			{
				screenPos.x = -screenPos.x;
				screenPos.y = -screenPos.y;
			}

			const bool inFrustum = frustum.IsPointInside(targetPos);
			const ArrowInfo info = inFrustum
				? CalcOverheadArrow(screenPos)
				: CalcEdgeArrow(screenPos);

			menu->SetArrowScreenPos(info.screenPos);
			menu->SetArrowAngleRad(info.angleRad);
			menu->SetVisible(info.visible);
			menu->SetPulsing(false);

			m_arrowPackets[i].Update();
		}
	}


	TutorialController::ArrowInfo TutorialController::CalcEdgeArrow(
		const Vector2& worldScreenPos) const
	{
		ArrowInfo info;
		info.visible = true;

		const float angle = atan2f(worldScreenPos.y, worldScreenPos.x);
		info.screenPos = Vector2(
			CIRCLE_RADIUS * cosf(angle),
			CIRCLE_CENTER_Y + CIRCLE_RADIUS * sinf(angle)
		);
		info.angleRad = angle + ARROW_ROT_OFFSET;

		return info;
	}


	TutorialController::ArrowInfo TutorialController::CalcOverheadArrow(
		const Vector2& worldScreenPos) const
	{
		ArrowInfo info;
		info.visible = true;
		info.screenPos = Vector2(worldScreenPos.x, worldScreenPos.y + OVERHEAD_OFFSET_Y);
		info.angleRad = OVERHEAD_ANGLE;
		return info;
	}
}
