/**
 * @file FormationController.cpp
 * @brief 陣形の切り替えと座標計算を管理するコントローラー
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationController.h"
#include "Formations.h"


namespace app
{
	namespace actor
	{
		FormationController::~FormationController() = default;


		FormationController::FormationController()
		{
			m_formations[static_cast<size_t>(EnFormationType::Circle)]   = std::make_unique<CircleFormation>();
			m_formations[static_cast<size_t>(EnFormationType::Triangle)] = std::make_unique<TriangleFormation>();
			m_formations[static_cast<size_t>(EnFormationType::Cluster)]  = std::make_unique<ClusterFormation>();
			m_formations[static_cast<size_t>(EnFormationType::Scatter)]  = std::make_unique<ScatterFormation>();

			m_currentFormation = m_formations[static_cast<size_t>(EnFormationType::Circle)].get();
		}


		void FormationController::CalculatePositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int count
		)
		{
			if (m_currentFormation == nullptr) return;

			// 現在の陣形に座標計算を委譲
			m_currentFormation->CalculatePositions(center, forward, out, count);

			// フォロワー数に応じて陣形レベルを更新
			const int newLevel = count / FOLLOWERS_PER_LEVEL;
			if (newLevel > m_formationLevel && m_onLevelUp){
				m_onLevelUp(newLevel);
			}
			m_formationLevel = newLevel;
		}


		void FormationController::SwitchFormation(EnFormationType type)
		{
			m_currentType      = type;
			m_currentFormation = m_formations[static_cast<size_t>(type)].get();
		}


		float FormationController::GetSpeedMultiplier() const
		{
			return m_currentFormation
				? m_currentFormation->GetSpeedMultiplier(m_formationLevel)
				: 1.0f;
		}


		void FormationController::CalculateNextLevelPositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int occupied
		)
		{
			if (!m_currentFormation) return;

			// 現在の m_outerRadius を保存（呼び出し後に復元する）
			const float savedRadius = m_currentFormation->GetOuterRadius();

			// 現在充填中のリング k の末尾インデックスを求める
			// リング k のスロット数 = k * FOLLOWERS_PER_LEVEL
			// リング 1〜k-1 の累計 = 1+2+...+(k-1) のスロット数 = ringStart
			int ringStart = 0;
			int k = 1;
			while (ringStart + k * FOLLOWERS_PER_LEVEL <= occupied)
			{
				ringStart += k * FOLLOWERS_PER_LEVEL;
				k++;
			}
			const int nextCount = ringStart + k * FOLLOWERS_PER_LEVEL;

			out.clear();
			m_currentFormation->CalculatePositions(center, forward, out, nextCount);
			m_currentFormation->SetOuterRadius(savedRadius);
		}


		float FormationController::GetOuterRadius() const
		{
			return m_currentFormation 
				? m_currentFormation->GetOuterRadius()
				: 0.0f;
		}


		float FormationController::GetJoinRadius() const
		{
			return m_currentFormation 
				? m_currentFormation->GetJoinRadius()
				: 0.0f;
		}


		float FormationController::GetLeaveRadius() const
		{
			return m_currentFormation 
				? m_currentFormation->GetLeaveRadius()
				: 0.0f;
		}
	}
}
