/**
 * @file DangerArrowSystem.cpp
 * @brief 危険矢印UIのシステムクラス実装
 */
#include "stdafx.h"
#include "DangerArrowSystem.h"

#include <algorithm>
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Orca/OrcaController.h"
#include "Source/Actor/Character/Orca/OrcaManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Graphics/Camera/SubCameraManager.h"


namespace app
{
	namespace ui
	{
		void DangerArrowSystem::Initialize()
		{
			for (auto& packet : m_packets)
			{
				packet.Initialize("Assets/parameter/UI/dangerArrow/DangerArrow.json");
			}
			for (auto& packet : m_orcaPackets)
			{
				packet.Initialize("Assets/parameter/UI/dangerArrow/DangerArrow.json");
			}
			SubCameraManager::Get().SetSpriteScale(SUB_VIEW_SCALE);
		}


		void DangerArrowSystem::Update()
		{
			RefreshTargetCache();
			CalcArrowInfos();
			UpdateSubView();

			// サブビューが表示中のとき、最近傍の矢印にパルスフラグを立てる
			int nearestArrowIdx = -1;
			if (SubCameraManager::Get().IsActive() && !m_arrowInfos.empty())
			{
				float minDist = m_arrowInfos[0].distSq;
				nearestArrowIdx = 0;
				for (int i = 1; i < static_cast<int>(m_arrowInfos.size()); i++)
				{
					if (m_arrowInfos[i].distSq < minDist)
					{
						minDist = m_arrowInfos[i].distSq;
						nearestArrowIdx = i;
					}
				}
			}

			for (int i = 0; i < MAX_ARROWS; i++)
			{
				auto* menu = m_packets[i].GetMenu();
				if (!menu) continue;

				if (i < static_cast<int>(m_arrowInfos.size()))
				{
					const auto& info = m_arrowInfos[i];
					menu->SetArrowScreenPos(info.screenPos);
					menu->SetArrowAngleRad(info.angleRad);
					menu->SetVisible(info.visible);
					menu->SetPulsing(i == nearestArrowIdx);
				}
				else
				{
					menu->SetVisible(false);
					menu->SetPulsing(false);
				}

				m_packets[i].Update();
			}

			// シャチの矢印（クマとは別枠。強調はしない）
			CalcOrcaArrowInfos();
			for (int i = 0; i < MAX_ORCA_ARROWS; i++)
			{
				auto* menu = m_orcaPackets[i].GetMenu();
				if (!menu) continue;

				if (i < static_cast<int>(m_orcaArrowInfos.size()))
				{
					const auto& info = m_orcaArrowInfos[i];
					menu->SetArrowScreenPos(info.screenPos);
					menu->SetArrowAngleRad(info.angleRad);
					menu->SetVisible(info.visible);
				}
				else
				{
					menu->SetVisible(false);
				}
				menu->SetPulsing(false);

				m_orcaPackets[i].Update();
			}
		}


		void DangerArrowSystem::Render(RenderContext& rc)
		{
			for (auto& packet : m_packets)
			{
				packet.Render(rc);
			}
			for (auto& packet : m_orcaPackets)
			{
				packet.Render(rc);
			}
		}


		void DangerArrowSystem::CalcOrcaArrowInfos()
		{
			m_orcaArrowInfos.clear();

			auto* orcaManager = actor::OrcaManager::GetInstance();
			if (orcaManager == nullptr) return;

			auto& mainCamera = CameraSystem::Get().GetMainCamera();
			const Vector3 camPos = mainCamera.GetPosition();
			const Vector3 camFwd = mainCamera.GetForward();
			const Frustum& frustum = g_renderingEngine->GetFrustum();

			for (const auto* controller : orcaManager->GetControllers())
			{
				if (static_cast<int>(m_orcaArrowInfos.size()) >= MAX_ORCA_ARROWS) break;
				if (controller == nullptr) continue;

				// 追跡中で、追っている子がいるときだけ矢印を出す（クマの CalcArrowInfos と同じ考え方）
				const auto* target = controller->GetFoundPenguin();
				if (target == nullptr || controller->GetState() != actor::OrcaController::EnState::Chase) continue;

				const Vector3 targetPos = target->GetTransform().m_position;

				// 以下、矢印の位置の出し方はクマの CalcArrowInfos と同じ
				ArrowInfo info;
				mainCamera.CalcScreenPositionFromWorldPosition(info.screenPos, targetPos);
				const Vector3 toTarget = targetPos - camPos;
				if (toTarget.Dot(camFwd) < 0.0f)
				{
					info.screenPos.x = -info.screenPos.x;
					info.screenPos.y = -info.screenPos.y;
				}
				const float distSq = toTarget.LengthSq();
				if (frustum.IsPointInside(targetPos))
				{
					info = CalcOverheadArrow(info.screenPos, distSq);
				}
				else
				{
					info = CalcEdgeArrow(info.screenPos, distSq);
				}
				m_orcaArrowInfos.push_back(info);
			}
		}


		void DangerArrowSystem::RefreshTargetCache()
		{
			const auto enemies = actor::EnemyManager::GetInstance()->GetEnemies();
			const auto controllers = actor::EnemyManager::GetInstance()->GetControllers();
			const int  enemyCount = static_cast<int>(enemies.size());

			if (static_cast<int>(m_enemySlots.size()) != enemyCount)
			{
				m_enemySlots.resize(enemyCount);
			}

			for (int i = 0; i < enemyCount; i++)
			{
				auto* enemy = enemies[i];
				auto* controller = controllers[i];
				if (!enemy || !controller) continue;

				const auto* found = controller->GetFoundPenguin();
				if (found != nullptr)
				{
					// チェイス中: キャッシュを更新
					m_enemySlots[i].cachedTarget = found;
				}
				else if (!enemy->GetEnemyStateMachine()->IsAttack())
				{
					// 攻撃中でもチェイス中でもない: キャッシュをクリア
					m_enemySlots[i].cachedTarget = nullptr;
				}
				// IsAttack() = true && found = null: 攻撃フェーズなのでキャッシュを保持
			}
		}


		void DangerArrowSystem::CalcArrowInfos()
		{
			m_arrowInfos.clear();

			const auto enemies = actor::EnemyManager::GetInstance()->GetEnemies();
			const int  enemyCount = static_cast<int>(enemies.size());

			auto& mainCamera = CameraSystem::Get().GetMainCamera();
			const Vector3 camPos = mainCamera.GetPosition();
			const Vector3 camFwd = mainCamera.GetForward();
			const Frustum& frustum = g_renderingEngine->GetFrustum();

			for (int i = 0; i < enemyCount && static_cast<int>(m_arrowInfos.size()) < MAX_ARROWS; i++)
			{
				auto* enemy = enemies[i];
				if (!enemy) continue;
				const auto* sm = enemy->GetEnemyStateMachine();
				if (!sm->IsChasing() && !sm->IsAttack()) continue;

				if (i >= static_cast<int>(m_enemySlots.size())) continue;
				const auto* target = m_enemySlots[i].cachedTarget;
				if (!target) continue;

				const Vector3 targetPos = target->GetTransform().m_position;

				// ワールド座標 → スクリーン座標 (info.screenPos に直接書き込む)
				ArrowInfo info;
				mainCamera.CalcScreenPositionFromWorldPosition(info.screenPos, targetPos);

				// カメラ背後にいる場合はスクリーン座標の符号が反転するため補正する
				const Vector3 toTarget = targetPos - camPos;
				if (toTarget.Dot(camFwd) < 0.0f)
				{
					info.screenPos.x = -info.screenPos.x;
					info.screenPos.y = -info.screenPos.y;
				}

				const float distSq = toTarget.LengthSq();
				const bool  inFrustum = frustum.IsPointInside(targetPos);

				// CalcEdge/Overhead は第1引数(ワールドスクリーン座標)を受け取り
				// 矢印配置座標を info.screenPos に上書きして返す。
				// 引数評価後に代入されるため info.screenPos の自己参照は安全。
				if (inFrustum)
				{
					info = CalcOverheadArrow(info.screenPos, distSq);
				}
				else
				{
					info = CalcEdgeArrow(info.screenPos, distSq);
				}

				m_arrowInfos.push_back(info);
			}
		}


		void DangerArrowSystem::UpdateSubView()
		{
			if (!SubCameraManager::Get().IsActive()) return;

			if (m_arrowInfos.empty())
			{
				SubCameraManager::Get().SetSpriteVisible(false);
				return;
			}

			// 最もカメラに近い攻撃対象を探す
			const auto& nearest = *std::min_element(
				m_arrowInfos.begin(), m_arrowInfos.end(),
				[](const ArrowInfo& a, const ArrowInfo& b) { return a.distSq < b.distSq; }
			);

			// フラスタム内（overhead）かつ近距離ならサブビューを非表示
			const float hideLimitSq = SUB_VIEW_HIDE_DIST * SUB_VIEW_HIDE_DIST;
			const bool  hideSubView = nearest.isOverhead && (nearest.distSq < hideLimitSq);

			SubCameraManager::Get().SetSpriteVisible(!hideSubView);

		}


		DangerArrowSystem::ArrowInfo DangerArrowSystem::CalcEdgeArrow(
			const Vector2& screenPos,
			const float distSq
		) const
		{
			ArrowInfo info;
			info.distSq     = distSq;
			info.isOverhead = false;
			info.visible    = true;
			info.screenPos  = CalcEdgeArrowScreenPos(screenPos);
			info.angleRad   = CalcEdgeArrowAngle(screenPos);
			return info;
		}


		DangerArrowSystem::ArrowInfo DangerArrowSystem::CalcOverheadArrow(
			const Vector2& screenPos,
			const float distSq
		) const
		{
			ArrowInfo info;
			info.distSq     = distSq;
			info.isOverhead = true;
			info.visible    = true;
			info.screenPos  = CalcOverheadArrowScreenPos(screenPos);
			info.angleRad   = ARROW_OVERHEAD_ANGLE;
			return info;
		}
	}
}
