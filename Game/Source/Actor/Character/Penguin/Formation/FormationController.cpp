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

			SwitchFormation(EnFormationType::Circle);
		}


		void FormationController::CalculatePositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int count,
			int countForLevel
		)
		{
			if (m_currentFormation == nullptr) return;

			// 現在の陣形に座標計算を委譲（count は m_outerRadius の算出に使われる）
			m_currentFormation->CalculatePositions(center, forward, out, count);

			// レベル判定は countForLevel で行う（-1 の場合は count を使う）
			const int effective = (countForLevel < 0) ? count : countForLevel;
			const int newLevel = effective / FOLLOWERS_PER_LEVEL;
			if (newLevel > m_formationLevel && m_onLevelUp){
				m_onLevelUp(newLevel);
			}
			m_formationLevel = newLevel;
		}


		void FormationController::SwitchFormation(EnFormationType type)
		{
			m_currentType      = type;
			m_currentFormation = m_formations[static_cast<size_t>(type)].get();

			// 陣形切り替えと同時にウルトチェーンも差し替える
			m_ultController.SetUlt(m_currentFormation->GetUlt(), ULT_DURATION, ULT_COOLDOWN);
		}


		float FormationController::GetSpeedMultiplier() const
		{
			if (!m_currentFormation) return 1.0f;

			// パッシブ倍率 × (ウルト中ならウルト倍率)
			float speed = m_currentFormation->GetPassive().GetSpeedMultiplier(m_formationLevel);
			if (m_ultController.IsActive())
			{
				speed *= m_currentFormation->GetUlt()->GetSpeedMultiplier(m_formationLevel);
			}
			return speed;
		}


		bool FormationController::HasWhirlpoolResistance() const
		{
			if (!m_currentFormation) return false;

			// パッシブ耐性 OR (ウルト中かつウルト耐性)
			if (m_currentFormation->GetPassive().HasWhirlpoolResistance()) return true;
			return m_ultController.IsActive()
				&& m_currentFormation->GetUlt()->HasWhirlpoolResistance();
		}


		float FormationController::GetJoinRadius(int count) const
		{
			return m_currentFormation ? m_currentFormation->GetJoinRadius(count) : 0.0f;
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
	}
}
