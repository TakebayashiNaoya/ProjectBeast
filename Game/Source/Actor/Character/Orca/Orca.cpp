/**
 * @file Orca.cpp
 * @brief シャチ（敵）
 */
#include "stdafx.h"
#include "Orca.h"
#include "OrcaParameter.h"
#include "OrcaSeaGrid.h"
#include "OrcaStateMachine.h"
#include "OrcaStatus.h"
#include "OrcaTypes.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** アニメーション。並びは EnOrcaAnimationType と一致させる */
			AnimationData ANIMATION_DATA[] =
			{
				{ "Assets/animData/orca/swim.tka", true },
				{ "Assets/animData/orca/mouth.tka", false },
			};
			static_assert(std::size(ANIMATION_DATA) == static_cast<size_t>(EnOrcaAnimationType::Num),
				"ANIMATION_DATA と EnOrcaAnimationType の数が合いません");

			ModelData ORCA_MODEL_DATA =
			{
				"Assets/modelData/orca/Orca.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				static_cast<uint8_t>(std::size(ANIMATION_DATA))
			};
		}


		Orca::Orca()
		{
			Init(ORCA_MODEL_DATA);
			GetModelRender().SetCastShadowInFarCascades(false);

			m_stateMachine = std::make_unique<OrcaStateMachine>(this);
			m_status = std::make_unique<OrcaStatus>();
			m_status->Setup();

			m_characterStateMachine = m_stateMachine.get();
		}


		Orca::~Orca()
		{}


		void Orca::SetSeaGrid(const OrcaSeaGrid* grid)
		{
			m_seaGrid = grid;
			m_stateMachine->SetSeaGrid(grid);
		}


		void Orca::Start()
		{
			CharacterBase::Start();

			// シャチは物理ボディを持たない。深い所には地形のコリジョンが無く、物理では浅瀬の手前で止められないため、
			// 海の中に留めるのは OrcaSeaGrid に任せる。カプセルは他のキャラクターとの衝突も起こさないよう外す
			m_characterController.RemoveRigidBoby();

			if (m_seaGrid != nullptr)
			{
				m_homeComponent = m_seaGrid->GetComponent(m_transform.m_position);
			}

			// カウントダウン中（UpdateModelOnly だけが呼ばれる間）も、縮めた大きさ・泳ぐ深さ・向きで見えるようにする
			m_stateMachine->InitializePose();
			m_transform.m_position = m_stateMachine->GetTransform().m_position;
			m_transform.m_rotation = m_stateMachine->GetTransform().m_rotation;
			m_transform.m_scale = m_stateMachine->GetTransform().m_scale;
		}


		void Orca::Update()
		{
			m_status->Update();
			m_stateMachine->Update();
			CheckInsideSea();

			CharacterBase::Update();
		}


		void Orca::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}


		void Orca::CheckInsideSea()
		{
			if (m_seaGrid == nullptr || !m_seaGrid->IsBuilt()) return;

			const Vector3 position = m_stateMachine->GetTransform().m_position;
			if (m_seaGrid->IsNavigable(position)) return;

			// ここに来るのは ResolveMove を通らずに位置が書き換えられたとき（あってはならない）
			m_seaViolationCount++;
			Vector3 back;
			if (m_seaGrid->FindNearestNavigable(position, m_homeComponent, back))
			{
				back.y = position.y;
				m_stateMachine->SetPosition(back);
			}
			if (auto* lm = GameLogManager::GetInstance())
			{
				lm->QueueEvent({ { "ev", "orca_sea_violation" }, { "orca_id", m_logId },
					{ "x", position.x }, { "z", position.z } });
			}
		}


		Vector3 Orca::GetSurfacePosition() const
		{
			return m_stateMachine->GetSurfacePosition();
		}


		Vector3 Orca::GetMouthPosition() const
		{
			return m_stateMachine->GetMouthPosition();
		}
	}
}
