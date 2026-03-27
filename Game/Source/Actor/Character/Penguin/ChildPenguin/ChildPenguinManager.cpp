/**
 * @file ChildPenguinManager.cpp
 * @brief 子ペンギンのマネージャー
 * @author 立山
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"


namespace app
{
	namespace actor
	{
		ChildPenguinManager* ChildPenguinManager::m_instance = nullptr;

		ChildPenguinManager::ChildPenguinManager()
		{

		}


		ChildPenguinManager::~ChildPenguinManager()
		{

		}


		void ChildPenguinManager::Start()
		{
			/** 各子ペンギンのStartを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->StartWrapper();
			}
		}

		void ChildPenguinManager::Update()
		{
			/** 子ペンギンが親と同じ移動方法で移動するため、親のステートを確認する */
			if (m_daddyPenguin)
			{
				auto* daddyState = m_daddyPenguin->GetStateMachine();

				/** 走っているかどうか */
				m_isDaddyRunning = daddyState->IsEqualCurrentState(PenguinRunState::ID());
				/** スライドしているかどうか */
				m_isDaddySliding = daddyState->IsEqualCurrentState(PenguinSlidingState::ID()) ||
					daddyState->IsEqualCurrentState(PenguinSlideStartState::ID());
				/** 飛び込んでいるかどうか */
				m_isDaddyDiving = daddyState->IsEqualCurrentState(PenguinDivingState::ID());
			}
			else
			{
				m_isDaddyRunning = m_isDaddySliding = m_isDaddyDiving = false;
			}

			/** 各子ペンギンのUpdateを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->UpdateWrapper();
			}

			/** 陣形の更新処理 */
			if (m_daddyPenguin != nullptr && !m_followers.empty())
			{
				/** 親の位置をベースに最大100個のポジションを計算 */
				CalculateFormationPositions();

				/** 隊列メンバーに割り当て */
				SortAndAssignFollowers();
			}
		}


		void ChildPenguinManager::Render(RenderContext& rc)
		{
			/** 各子ペンギンのRenderを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->RenderWrapper(rc);
			}
		}


		Vector3 ChildPenguinManager::GetDaddyPosition() const
		{
			if (m_daddyPenguin != nullptr)
			{
				return m_daddyPenguin->GetTransform().m_position;
			}
			return Vector3::Zero;
		}


		void ChildPenguinManager::CreateChildPenguin(const int childPenguinNum)
		{
			/** 既に子ペンギンがいる場合は追加で生成 */
			for (int i = 0; i < childPenguinNum; i++) {
				m_childPenguinList.push_back(new ChildPenguin);
			}
		}


		void ChildPenguinManager::AddFollower(ChildPenguin* penguin)
		{
			/** 既に登録されていないか確認してから追加 */
			auto it = std::find(m_followers.begin(), m_followers.end(), penguin);
			if (it == m_followers.end()) {
				m_followers.push_back(penguin);
			}
			/** メンバーが増えたので次フレームで再ソート・再割り当てが走る */
		}


		void ChildPenguinManager::RemoveFollower(ChildPenguin* penguin)
		{
			/** 登録されているか確認してから削除 */
			auto it = std::find(m_followers.begin(), m_followers.end(), penguin);
			if (it != m_followers.end()) {
				m_followers.erase(it);
			}
			/** メンバーが減ったので外側の子が内側に詰める処理が次フレームで自然に行われる */
		}


		void ChildPenguinManager::CalculateFormationPositions()
		{
			m_formationPositions.clear();

			/** 親の現在座標を取得 */
			Vector3 centerPos = m_daddyPenguin->GetTransform().m_position;

			int currentCount = 0;	/** 今何匹目を配置しているかのカウント */
			int layer = 1;			/** 階層カウント（1が一番内側の円） */

			/** 最大数に達するまで円を広げながら計算 */
			while (currentCount < MAX_FORMATION_COUNT)
			{
				/** 現在の階層の半径 */
				float r = FORMATION_BASE_RADIUS + (layer - 1) * FORMATION_RADIUS_STEP;

				/** 円周の長さ */
				float circumference = 2.0f * Math::PI * r;

				/** この階層に配置できる最大数(最低1匹は置く) */
				int maxInThisLayer = max(1, static_cast<int>(circumference / FORMATION_MIN_DISTANCE));

				/** 何°毎に配置するかの角度算出 */
				float angleStep = 360.0f / maxInThisLayer;

				/** この階層に配置する数だけループ */
				for (int i = 0; i < maxInThisLayer && currentCount < MAX_FORMATION_COUNT; ++i)
				{
					/* 角度を計算して、円周上の座標を求める */
					float angleDeg = i * angleStep;
					float angleRad = angleDeg * (Math::PI / 180.0f); // ラジアン変換

					Vector3 targetPos = centerPos;
					// Z軸とX軸で円を描く（ワールド座標系固定）
					targetPos.x += r * cosf(angleRad);
					targetPos.z += r * sinf(angleRad);
					// ※Y軸（高さ）は地形に沿わせる処理が別途必要になる場合があります

					m_formationPositions.push_back(targetPos);
					currentCount++;
				}
				layer++;
			}
		}


		void ChildPenguinManager::SortAndAssignFollowers()
		{
			/** 1. 隊列のソート */
			/** NOTE: std::stable_sortを使うことで「同じ条件なら元々の順番（参加順）を保つ」ことができる */
			std::stable_sort(m_followers.begin(), m_followers.end(), [](ChildPenguin* a, ChildPenguin* b) {
				bool aIsClingy = (a->GetChildPenguinType() == EnChildPenguinType::Clingy);
				bool bIsClingy = (b->GetChildPenguinType() == EnChildPenguinType::Clingy);

				/** aが甘えん坊でbが違うなら、aを前にする */
				if (aIsClingy && !bIsClingy) return true;
				/** その逆 */
				if (!aIsClingy && bIsClingy) return false;

				/** どちらも甘えん坊、あるいはどちらも甘えん坊以外の場合は順番を変えない（falseを返す） */
				return false;
				});

			/** 2. 目標座標の割り当て */
			/** 0番目（一番内側）から順番に割り当てていく */
			for (size_t i = 0; i < m_followers.size(); ++i)
			{
				if (i < m_formationPositions.size())
				{
					m_followers[i]->SetFormationTargetPosition(m_formationPositions[i]);
				}
			}
		}
	}
}