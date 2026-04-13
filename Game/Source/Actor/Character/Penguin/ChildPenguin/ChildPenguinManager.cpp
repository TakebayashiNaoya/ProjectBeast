/**
 * @file ChildPenguinManager.cpp
 * @brief 子ペンギンのマネージャー
 * @author 立山、竹林
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinManager.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinTypes.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Manager/IglooManager.h"
#include <random>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** スポーン座標を求めるレイの発射高度 */
			constexpr float SPAWN_RAY_START_Y = 1000.0f;
			/** 拒絶サンプリングの最大試行回数（無限ループ防止） */
			constexpr int SPAWN_MAX_RETRY = 100;
		}


		ChildPenguinManager* ChildPenguinManager::m_instance = nullptr;


		ChildPenguinManager::ChildPenguinManager()
		{}


		ChildPenguinManager::~ChildPenguinManager()
		{}


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

			/** DaddyPenguinに近い上位N匹を可聴対象として更新する */
			UpdateAudiblePenguins();

			/** 削除待ちのペンギンを安全に破棄する (遅延削除) */
			for (auto* deadPenguin : m_destroyList)
			{
				/** 管理リストから安全に取り除く */
				auto it = std::find(m_childPenguinList.begin(), m_childPenguinList.end(), deadPenguin);
				if (it != m_childPenguinList.end())
				{
					m_childPenguinList.erase(it);
				}

				/** 全ての Update 処理が終わったここで、初めてメモリを解放する */
				delete deadPenguin;
			}
			m_destroyList.clear();
		}


		void ChildPenguinManager::Render(RenderContext& rc)
		{
			/** 各子ペンギンのRenderを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->RenderWrapper(rc);
			}
		}


		void ChildPenguinManager::UpdateModelOnly()
		{
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->UpdateModelOnly();
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


		void ChildPenguinManager::CreateChildPenguins(
			int seriousNum,
			int clingyNum,
			int naughtyNum,
			int clumsyNum,
			int caringNum,
			float spawnRadius
		)
		{
			/** タイプと生成数のペアをまとめて処理する */
			const std::pair<EnChildPenguinType, int> spawnList[] =
			{
				{ EnChildPenguinType::Serious, seriousNum },
				{ EnChildPenguinType::Clingy,  clingyNum  },
				{ EnChildPenguinType::Naughty, naughtyNum },
				{ EnChildPenguinType::Clumsy,  clumsyNum  },
				{ EnChildPenguinType::Caring,  caringNum  },
			};

			for (const auto& [type, num] : spawnList)
			{
				for (int i = 0; i < num; i++)
				{
					SpawnOne(type, spawnRadius);
				}
			}
		}


		void ChildPenguinManager::SpawnOne(EnChildPenguinType type, float spawnRadius)
		{
			/** 円内のランダムな座標を生成 */
			const Vector3 xzPos = GenerateRandomSpawnPosition(spawnRadius);

			/** レイキャストで地面のyを取得 */
			const float groundY = GetGroundY(xzPos.x, xzPos.z);
			const Vector3 spawnPos = Vector3(xzPos.x, groundY, xzPos.z);

			/** 子ペンギンを生成してタイプと座標をセット */
			CreateChildPenguin();
			auto* child = m_childPenguinList.back();
			child->SetChildPenguinType(type);
			child->SetPosition(spawnPos);
			child->GetStateMachine()->SetPosition(spawnPos);
			child->StartWrapper();
		}


		Vector3 ChildPenguinManager::GenerateRandomSpawnPosition(float radius)
		{
			static std::mt19937 engine(std::random_device{}());
			std::uniform_real_distribution<float> dist(-radius, radius);

			/** 拒絶サンプリング：円の外側に落ちた点を棄却して再抽選する */
			for (int i = 0; i < SPAWN_MAX_RETRY; i++)
			{
				const float x = dist(engine);
				const float z = dist(engine);

				if ((x * x + z * z) <= (radius * radius))
				{
					return Vector3(x, 0.0f, z);
				}
			}

			/** 最大試行回数を超えた場合は原点付近に置く */
			return Vector3::Zero;
		}


		float ChildPenguinManager::GetGroundY(float x, float z)
		{
			const Vector3 rayStart = Vector3(x, SPAWN_RAY_START_Y, z);
			const Vector3 rayEnd = Vector3(x, -10.0f, z);

			nsBeastEngine::nsCollision::RaycastHit hit;
			const bool isHit = nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(
				rayStart,
				rayEnd,
				hit
			);

			/** ヒットした場合は衝突点のyを返す。ヒットしなければ海面（y=0.0f）を返す */
			return isHit ? hit.point.y : 0.0f;
		}


		void ChildPenguinManager::CreateChildPenguin()
		{
			m_childPenguinList.push_back(new ChildPenguin);
		}


		void ChildPenguinManager::RemoveAndDestroy(ChildPenguin* penguin)
		{
			/** 隊列から取り除く */
			RemoveFollower(penguin);

			/** ステージ上のペンギン総数を減らす */
			ScoreManager::GetInstance().SubTotalCount();

			/** 状態管理セットからも取り除く */
			m_downingPenguins.erase(penguin);
			m_attemptingPenguins.erase(penguin);
			m_roamingPenguins.erase(penguin);
			m_assignedTargets.erase(penguin);

			/** 可聴セットからも取り除く */
			m_audiblePenguins.erase(penguin);

			/** 即座に m_childPenguinList から erase したり delete したりせず、 */
			/** 削除予定リストに登録するだけに留める */
			auto it = std::find(m_destroyList.begin(), m_destroyList.end(), penguin);
			if (it == m_destroyList.end())
			{
				m_destroyList.push_back(penguin);
			}
		}


		void ChildPenguinManager::AddFollower(ChildPenguin* penguin)
		{
			/** 既に登録されていないか確認してから追加 */
			auto it = std::find(m_followers.begin(), m_followers.end(), penguin);
			if (it == m_followers.end()) {
				m_followers.push_back(penguin);
				ScoreManager::GetInstance().AddCollectedCount();
			}
			/** メンバーが増えたので次フレームで再ソート・再割り当てが走る */
		}


		void ChildPenguinManager::RemoveFollower(ChildPenguin* penguin)
		{
			/** 登録されているか確認してから削除 */
			auto it = std::find(m_followers.begin(), m_followers.end(), penguin);
			if (it != m_followers.end()) {
				m_followers.erase(it);
				ScoreManager::GetInstance().SubCollectedCount();
			}
			/** メンバーが減ったので外側の子が内側に詰める処理が次フレームで自然に行われる */
		}


		int ChildPenguinManager::GetRescuedNum() const
		{
			if (m_daddyPenguin == nullptr) return 0;

			int count = 0;
			const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;

			for (const auto* cp : m_childPenguinList)
			{
				if (!cp) continue;

				/** 親との水平距離を計算 */
				Vector3 diff = daddyPos - cp->GetTransform().m_position;
				diff.y = 0.0f;
				const float dist = diff.Length();

				/** 各子ペンギンの joinDistance 以内なら救出済みとみなす */
				if (dist <= cp->GetJoinDistance())
				{
					count++;
				}
			}
			return count;
		}


		void ChildPenguinManager::CalculateFormationPositions()
		{
			m_formationPositions.clear();

			/** 親の現在座標を取得 */
			Vector3 centerPos = m_daddyPenguin->GetTransform().m_position;

			int currentCount = 0; /** 今何匹目を配置しているかのカウント */
			int layer = 1;        /** 階層カウント（1が一番内側の円） */

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
					/** 角度を計算して、円周上の座標を求める */
					float angleDeg = i * angleStep;
					float angleRad = angleDeg * (Math::PI / 180.0f); /** ラジアン変換 */

					Vector3 targetPos = centerPos;
					/** Z軸とX軸で円を描く（ワールド座標系固定） */
					targetPos.x += r * cosf(angleRad);
					targetPos.z += r * sinf(angleRad);
					/** ※Y軸（高さ）は地形に沿わせる処理が別途必要になる場合があります */

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


		void ChildPenguinManager::StartIglooEvent(const Vector3& interactPos)
		{
			// イベント開始時に、連れ歩いている子ペンギンの数をカウントにセットする
			m_iglooEnteringCount = static_cast<int>(m_followers.size());

			// 全員に「入り口へ向かえ！」と命令を出す
			for (auto* child : m_followers)
			{
				if (child && child->GetAIController())
				{
					child->GetAIController()->StartEnterIglooEvent(interactPos);
				}
			}
		}


		void ChildPenguinManager::FinishEnterIglooOne()
		{
			// 報告を受けるたびにカウントを1減らす
			m_iglooEnteringCount--;
		}


		bool ChildPenguinManager::IsIglooEventFinished() const
		{
			// カウントが0以下になったら全員入り終わったと判定
			return m_iglooEnteringCount <= 0;
		}


		void ChildPenguinManager::EndIglooEvent(const Vector3& exitPos)
		{
			// 全ての子ペンギンをチェックし、イベントに参加している子全員をリセットする
			for (auto* child : m_childPenguinList)
			{
				if (child && child->GetAIController())
				{
					// ★ 修正：中に入っているかではなく「イベント命令を受けているか」で判定！
					// これで、まだ歩いている途中の子も全員キャンセルされて外にワープします！
					if (child->GetAIController()->IsEnterIglooMode())
					{
						child->GetAIController()->EndEnterIglooEvent(exitPos);
					}
				}
			}
      
      
		//============================================//
		// サウンド：近傍ペンギンの可聴管理
		//============================================//

		void ChildPenguinManager::UpdateAudiblePenguins()
		{
			m_audiblePenguins.clear();

			/** DaddyPenguinがいなければ全員不可聴にして終わる */
			if (m_daddyPenguin == nullptr) return;

			const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;

			/** 有効な子ペンギンを距離付きで収集する */
			std::vector<std::pair<float, ChildPenguin*>> distList;
			distList.reserve(m_childPenguinList.size());

			for (auto* cp : m_childPenguinList)
			{
				if (!cp) continue;

				Vector3 diff = cp->GetTransform().m_position - daddyPos;
				diff.y = 0.0f;
				const float distSq = diff.LengthSq();
				distList.emplace_back(distSq, cp);
			}

			/** 距離の昇順でソートし、上位 AUDIBLE_PENGUIN_NUM 匹を可聴対象に登録する */
			std::sort(distList.begin(), distList.end(),
				[](const std::pair<float, ChildPenguin*>& a, const std::pair<float, ChildPenguin*>& b)
				{
					return a.first < b.first;
				});

			const int audibleCount = min(static_cast<int>(distList.size()), AUDIBLE_PENGUIN_NUM);
			for (int i = 0; i < audibleCount; ++i)
			{
				m_audiblePenguins.insert(distList[i].second);
			}
		}


		bool ChildPenguinManager::IsAudible(const ChildPenguin* penguin) const
		{
			return m_audiblePenguins.count(const_cast<ChildPenguin*>(penguin)) > 0;
		}


		//============================================//
		// 世話焼き用：問題行動ペンギンの状態管理
		//============================================//

		void ChildPenguinManager::RegisterDowning(ChildPenguin* penguin)
		{
			m_downingPenguins.insert(penguin);
		}


		void ChildPenguinManager::UnregisterDowning(ChildPenguin* penguin)
		{
			m_downingPenguins.erase(penguin);
		}


		bool ChildPenguinManager::IsDowning(const ChildPenguin* penguin) const
		{
			return m_downingPenguins.count(const_cast<ChildPenguin*>(penguin)) > 0;
		}


		void ChildPenguinManager::RegisterAttempting(ChildPenguin* penguin)
		{
			m_attemptingPenguins.insert(penguin);
		}


		void ChildPenguinManager::UnregisterAttempting(ChildPenguin* penguin)
		{
			m_attemptingPenguins.erase(penguin);
		}


		bool ChildPenguinManager::IsAttempting(const ChildPenguin* penguin) const
		{
			return m_attemptingPenguins.count(const_cast<ChildPenguin*>(penguin)) > 0;
		}


		void ChildPenguinManager::RegisterRoaming(ChildPenguin* penguin)
		{
			m_roamingPenguins.insert(penguin);
		}


		void ChildPenguinManager::UnregisterRoaming(ChildPenguin* penguin)
		{
			m_roamingPenguins.erase(penguin);
		}


		bool ChildPenguinManager::IsRoaming(const ChildPenguin* penguin) const
		{
			return m_roamingPenguins.count(const_cast<ChildPenguin*>(penguin)) > 0;
		}


		void ChildPenguinManager::RegisterAssigned(ChildPenguin* penguin)
		{
			m_assignedTargets.insert(penguin);
		}


		void ChildPenguinManager::UnregisterAssigned(ChildPenguin* penguin)
		{
			m_assignedTargets.erase(penguin);
		}


		ChildPenguin* ChildPenguinManager::FindNearestDowning(
			const Vector3& from,
			const std::unordered_set<ChildPenguin*>& excludeSet,
			float maxRange
		) const
		{
			ChildPenguin* nearest = nullptr;
			const float maxRangeSq = maxRange * maxRange;
			float minDistSq = maxRangeSq;

			for (auto* penguin : m_downingPenguins)
			{
				/** 既に他の世話焼きが担当しているペンギンはスキップする */
				if (excludeSet.count(penguin) > 0) continue;

				Vector3 diff = penguin->GetTransform().m_position - from;
				diff.y = 0.0f;
				const float distSq = diff.LengthSq();

				/** 最大距離より遠ければスキップする */
				if (distSq > maxRangeSq) continue;

				if (distSq < minDistSq)
				{
					minDistSq = distSq;
					nearest = penguin;
				}
			}

			return nearest;
		}


		ChildPenguin* ChildPenguinManager::FindNearestNeedingSupervision(
			const Vector3& from,
			const std::unordered_set<ChildPenguin*>& excludeSet,
			float maxRange
		) const
		{
			ChildPenguin* nearest = nullptr;
			const float maxRangeSq = maxRange * maxRange;
			float minDistSq = maxRangeSq;

			/** 甘えん坊と徘徊中のやんちゃを合わせて最近傍を探す */
			auto checkSet = [&](const std::unordered_set<ChildPenguin*>& targetSet)
				{
					for (auto* penguin : targetSet)
					{
						if (excludeSet.count(penguin) > 0) continue;

						Vector3 diff = penguin->GetTransform().m_position - from;
						diff.y = 0.0f;
						const float distSq = diff.LengthSq();

						/** 最大距離より遠ければスキップする */
						if (distSq > maxRangeSq) continue;

						if (distSq < minDistSq)
						{
							minDistSq = distSq;
							nearest = penguin;
						}
					}
				};

			checkSet(m_attemptingPenguins);
			checkSet(m_roamingPenguins);

			return nearest;
		}
	}
}