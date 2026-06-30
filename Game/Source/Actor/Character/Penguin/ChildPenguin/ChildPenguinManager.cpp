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
#include "Source/Manager/InGameUIManager.h"
#include "Source/UI/CPReaction/CPReactionMenu.h"
#include "Source/UI/CPReaction/CPReactionSystem.h"
#include "Source/UI/RemainingChild/RemainingChildMenu.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
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
			/** 群れの広さ（半径） */
			constexpr float CLUSTER_RADIUS = 150.0f;
			/** 群れサイズの最小値 */
			constexpr int CLUSTER_SIZE_MIN = 1;
			/** 群れサイズの最大値 */
			constexpr int CLUSTER_SIZE_MAX = 5;

			//-----------ゴーストペンギン関連---------------//
			/** 非表示待機時間 */
			constexpr float GHOST_HIDDEN_WAIT_TIME = 2.0f;
			/** かかった時間 */
			constexpr float GHOST_TOTAL_TIME = 7.0f;
			/** ゴーストペンギンのモデルパス */
			constexpr const char* GHOST_MODEL_PATH = "Assets/modelData/penguin/childPenguin/GhostChildPenguin.tkm";
			/** 一オフセットY */
			constexpr float GHOST_POSITION_OFFSET_Y = 0.2f;
			// 浮上アニメーション持続時間。
			constexpr float GHOST_SURFACING_ANIM_DURATION = 3.0f;
			/** スケールアップ */
			const Vector3 GHOST_SCALE_UP = Vector3(0.8f, 1.0f, 1.0f);
			/** 方向正規化の平方 */
			constexpr float GHOST_DIR_NORMALIZE_SQ = 0.0001f;
		}


		ChildPenguinManager* ChildPenguinManager::m_instance = nullptr;


		ChildPenguinManager::ChildPenguinManager()
			: m_ghostPenguinNum(0)
			, m_isGhostHidden(false)
			, m_ghostTimer(0.0f)
			, m_target(nullptr)
		{}


		ChildPenguinManager::~ChildPenguinManager()
		{
			g_renderingEngine->UnregisterCustomRenderer(&m_rangeVisualizer);
		}


		void ChildPenguinManager::Start()
		{
			m_rangeVisualizer.Init();
			g_renderingEngine->RegisterCustomRenderer(&m_rangeVisualizer);

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

			UpdateGhostPenguins();

			/** L1/R1 で陣形を循環切り替え */
			if (g_pad[0]->IsTrigger(enButtonRB1))
			{
				const int next = (static_cast<int>(m_formationController.GetCurrentType()) + 1) % static_cast<int>(EnFormationType::Num);
				m_formationController.SwitchFormation(static_cast<EnFormationType>(next));
			}
			else if (g_pad[0]->IsTrigger(enButtonLB1))
			{
				const int prev = (static_cast<int>(m_formationController.GetCurrentType()) + static_cast<int>(EnFormationType::Num) - 1) % static_cast<int>(EnFormationType::Num);
				m_formationController.SwitchFormation(static_cast<EnFormationType>(prev));
			}

			/** 陣形の更新処理 */
			if (m_daddyPenguin != nullptr)
			{
				if (!m_followers.empty())
				{
					/** 親の位置をベースに最大100個のポジションを計算 */
					CalculateFormationPositions();

					/** 隊列メンバーに割り当て */
					SortAndAssignFollowers();
				}
				else
				{
					/** フォロワーが0匹でもレベル0の最小半径を確定させるため1スロット分だけ計算する */
					Vector3 forward = Vector3::Front;
					m_daddyPenguin->GetTransform().m_rotation.Apply(forward);
					m_formationPositions.clear();
					m_formationController.CalculatePositions(
						m_daddyPenguin->GetTransform().m_position, forward, m_formationPositions, 1);
				}
			}

			/** 陣形範囲ビジュアライザーの更新 */
			m_rangeVisualizer.SetVisible(true);
			if (m_daddyPenguin != nullptr)
			{
				const Vector3 center   = m_daddyPenguin->GetTransform().m_position;
				const float joinRadius = m_formationController.GetJoinRadius();
				/** 次レベルの空きスロットを計算（CalculateNextLevelPositions内でm_outerRadiusが一時変化するため先に読んでおく） */
				CalculateNextLevelSlots(center);
				m_rangeVisualizer.Update(center, joinRadius, m_nextLevelSlots);
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

			RenderGhostPenguins(rc);
		}


		void ChildPenguinManager::UpdateModelOnly()
		{
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->UpdateAtCountDownTime();
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
			// ==========================================
			// 生成するすべてのペンギンの「タイプ」を1つのリスト（プール）にまとめる
			// ==========================================
			std::vector<EnChildPenguinType> spawnPool;
			// メモリ確保の最適化（全匹分のサイズをあらかじめ確保）
			spawnPool.reserve(seriousNum + clingyNum + naughtyNum + clumsyNum + caringNum);

			// リストに指定された数だけタイプを追加していく
			for (int i = 0; i < seriousNum; i++) spawnPool.push_back(EnChildPenguinType::Serious);
			for (int i = 0; i < clingyNum; i++)  spawnPool.push_back(EnChildPenguinType::Clingy);
			for (int i = 0; i < naughtyNum; i++) spawnPool.push_back(EnChildPenguinType::Naughty);
			for (int i = 0; i < clumsyNum; i++)  spawnPool.push_back(EnChildPenguinType::Clumsy);
			for (int i = 0; i < caringNum; i++)  spawnPool.push_back(EnChildPenguinType::Caring);

			// ==========================================
			// リストの中身をランダムにシャッフルする
			// ==========================================
			static std::mt19937 engine(std::random_device{}());
			std::shuffle(spawnPool.begin(), spawnPool.end(), engine);

			// ==========================================
			// クラスター（群れ）として配置していく
			// ==========================================

			// 3〜5匹の間でランダムにサイズを決めるための設定
			std::uniform_int_distribution<int> clusterSizeDist(CLUSTER_SIZE_MIN, CLUSTER_SIZE_MAX);
			int currentMaxClusterSize = 0; // 現在作ろうとしている群れの目標サイズ

			Vector3 currentClusterCenter = Vector3::Zero;
			int currentClusterCount = 0;

			// シャッフルされたリストから1匹ずつ順番に取り出して配置する
			for (EnChildPenguinType type : spawnPool)
			{
				// 群れの人数が0、または目標のサイズに達したら、新しい群れ（リーダー位置とサイズ）を決める
				if (currentClusterCount == 0 || currentClusterCount >= currentMaxClusterSize)
				{
					// 新しい中心位置を決める
					currentClusterCenter = GenerateRandomSpawnPosition(spawnRadius);
					currentClusterCount = 0;

					// ★ 次に作る群れのサイズを 1〜5 匹の間でランダムに決定する
					currentMaxClusterSize = clusterSizeDist(engine);
				}

				// 決まった群れの中心を渡して1匹生成
				SpawnOne(type, spawnRadius, currentClusterCenter, CLUSTER_RADIUS);

				currentClusterCount++;
			}
		}


		void ChildPenguinManager::SpawnOne(EnChildPenguinType type, float spawnRadius, const Vector3& clusterCenter, float clusterRadius)
		{
			/** 円内のランダムな座標を生成 */
			Vector3 xzPos;

			if (clusterCenter.x == 0.0f && clusterCenter.y == 0.0f && clusterCenter.z == 0.0f)
			{
				xzPos = GenerateRandomSpawnPosition(spawnRadius);
			}
			else
			{
				xzPos = GenerateClusterMemberPosition(clusterCenter, clusterRadius);
			}

			/** レイキャストで地面のyを取得 */
			const float groundY = GetGroundY(xzPos.x, xzPos.z);
			const Vector3 spawnPos = Vector3(xzPos.x, groundY, xzPos.z);

			/** 子ペンギンを生成してタイプと座標をセット */
			CreateChildPenguin();
			auto* child = m_childPenguinList.back();
			child->SetLogId(m_nextLogId++);
			child->SetChildPenguinType(type);
			child->SetPosition(spawnPos);
			child->GetStateMachine()->SetPosition(spawnPos);
			child->StartWrapper();
			if (auto* lm = GameLogManager::GetInstance())
				lm->RecordSpawn("penguin", child->GetLogId(), { {"type", child->GetChildPenguinTypeStr()} });
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


		Vector3 ChildPenguinManager::GenerateClusterMemberPosition(const Vector3& center, float clusterRadius)
		{
			static std::mt19937 engine(std::random_device{}());
			std::uniform_real_distribution<float> dist(-clusterRadius, clusterRadius);

			for (int i = 0; i < SPAWN_MAX_RETRY; i++)
			{
				const float x = dist(engine);
				const float z = dist(engine);

				// 群れの半径の円内に収まっているかチェック
				if ((x * x + z * z) <= (clusterRadius * clusterRadius))
				{
					// 群れの中心座標にオフセットを足して返す
					return Vector3(center.x + x, 0.0f, center.z + z);
				}
			}

			// 試行回数を超えた場合は安全のため中心位置をそのまま返す
			return center;
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
			if (auto* lm = GameLogManager::GetInstance())
			{
				lm->QueueEvent({ {"ev", "penguin_die"}, {"penguin_id", penguin->GetLogId()} });
				lm->RecordDespawn("penguin", penguin->GetLogId(), "dead");
			}

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
				InGameUIManager::GetInstance()->GetCPReactionSystem()->SetTarget(penguin, ui::EnReactionType::Happy);

				if (auto* menu = InGameUIManager::GetInstance()->GetRemainingChildMenu())
				{
					menu->SetTarget(penguin);
				}

				if (auto* lm = GameLogManager::GetInstance())
					lm->QueueEvent({ {"ev", "penguin_join"}, {"penguin_id", penguin->GetLogId()} });
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
				InGameUIManager::GetInstance()->GetCPReactionSystem()->SetTarget(penguin, ui::EnReactionType::Trouble);
				if (auto* lm = GameLogManager::GetInstance())
					lm->QueueEvent({ {"ev", "penguin_leave"}, {"penguin_id", penguin->GetLogId()} });
			}
			/** メンバーが減ったので外側の子が内側に詰める処理が次フレームで自然に行われる */
		}


		bool ChildPenguinManager::IsFollower(const ChildPenguin* penguin) const
		{
			auto it = std::find(m_followers.begin(), m_followers.end(), penguin);
			return it != m_followers.end();
		}


		int ChildPenguinManager::GetClingyCount() const
		{
			// 甘えん坊の数をカウントする。
			int clingyCount = 0;

			// 現在の隊列の中に甘えん坊がいるかどうかをチェックする。
			for (const auto& penguin : m_followers)
			{
				// 甘えん坊ならカウントアップ。
				if (penguin->GetChildPenguinType() == EnChildPenguinType::Clingy)
				{
					clingyCount++;
				}
			}
			return clingyCount;
		}


		int ChildPenguinManager::GetRescuedNum() const
		{
			return static_cast<int>(m_followers.size());
		}


		void ChildPenguinManager::CalculateFormationPositions()
		{
			const Vector3 center  = m_daddyPenguin->GetTransform().m_position;

			/** 親の向きから前方ベクトルを取得 */
			Vector3 forward = Vector3::Front;
			m_daddyPenguin->GetTransform().m_rotation.Apply(forward);

			/** 現在のフォロワー数分だけ座標を生成する */
			m_formationPositions.clear();
			m_formationController.CalculatePositions(
				center,
				forward,
				m_formationPositions,
				static_cast<int>(m_followers.size())
			);
		}


		void ChildPenguinManager::CalculateNextLevelSlots(const Vector3& center)
		{
			/** 親の向きから前方ベクトルを取得 */
			Vector3 forward = Vector3::Front;
			m_daddyPenguin->GetTransform().m_rotation.Apply(forward);

			/** 次レベル分の全座標を計算（m_outerRadiusは内部で復元される） */
			std::vector<Vector3> allNextLevelPositions;
			m_formationController.CalculateNextLevelPositions(center, forward, allNextLevelPositions, static_cast<int>(m_followers.size()));

			/** 現在フォロワーが占めているスロットを除き、空きスロットだけを格納する */
			m_nextLevelSlots.clear();
			const size_t occupied = m_followers.size();
			for (size_t i = occupied; i < allNextLevelPositions.size(); ++i)
			{
				m_nextLevelSlots.push_back(allNextLevelPositions[i]);
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


		/**
		 * @brief かまくらイベントを開始する
		 * @param interactPos 向かうべきかまくらの入り口（青い円）の座標
		 */
		void ChildPenguinManager::StartIglooEvent(const Vector3& interactPos)
		{
			// イベントに参加させるペンギンを一時的に格納するリスト
			std::vector<ChildPenguin*> targetPenguins;

			// 現在「隊列」にいるペンギン（m_followers）を無条件で全員追加
			for (auto* child : m_followers)
			{
				if (child) targetPenguins.push_back(child);
			}

			// 隊列から一時的に外れているが、親の近くにいるペンギンも追加
			if (m_daddyPenguin != nullptr)
			{
				const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;

				for (auto* child : m_childPenguinList)
				{
					if (!child) continue;

					// リストに入っている子（隊列内の子）はスキップ
					auto it = std::find(targetPenguins.begin(), targetPenguins.end(), child);
					if (it != targetPenguins.end()) continue;

					// 隊列にいないペンギンについて、親との水平距離を計算
					Vector3 diff = daddyPos - child->GetTransform().m_position;
					diff.y = 0.0f;
					float distToDaddy = diff.Length();

					// 入隊半径（陣形設定）の範囲にいる子ペンギンを呼ぶ
					if (distToDaddy <= GetJoinRadius())
					{
						targetPenguins.push_back(child);
					}
				}
			}

			// ターゲットになったペンギンの総数をカウントにセットする
			m_iglooEnteringCount = static_cast<int>(targetPenguins.size());

			// 全員に「入り口へ向かえ！」と命令を出す
			for (auto* child : targetPenguins)
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

			/** キャッシュを再利用してヒープ確保を避ける */
			m_audibleDistCache.clear();

			for (auto* cp : m_childPenguinList)
			{
				if (!cp) continue;

				Vector3 diff = cp->GetTransform().m_position - daddyPos;
				diff.y = 0.0f;
				m_audibleDistCache.emplace_back(diff.LengthSq(), cp);
			}

			/** nth_element で上位 AUDIBLE_PENGUIN_NUM 匹だけ O(n) で抽出する */
			const int audibleCount = min(static_cast<int>(m_audibleDistCache.size()), AUDIBLE_PENGUIN_NUM);
			std::nth_element(
				m_audibleDistCache.begin(),
				m_audibleDistCache.begin() + audibleCount,
				m_audibleDistCache.end(),
				[](const std::pair<float, ChildPenguin*>& a, const std::pair<float, ChildPenguin*>& b)
				{
					return a.first < b.first;
				});

			for (int i = 0; i < audibleCount; ++i)
			{
				m_audiblePenguins.insert(m_audibleDistCache[i].second);
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


		ChildPenguinManager::GhostPenguinInfo::GhostPenguinInfo()
			: modelRender()
			, floatCurve()
			, target(nullptr)
			, timer(0.0f)
			, isHidden(false)
			, isDebuffActive(false)
			, position(Vector3::Zero)
		{}


		void ChildPenguinManager::RegisterGhostPenguin(ChildPenguin* penguin, Enemy* target)
		{
			const Vector3 dethPos = penguin->GetTransform().m_position;
			const Quaternion dethRot = penguin->GetTransform().m_rotation;
			const Vector3 dethScale = penguin->GetTransform().m_scale + GHOST_SCALE_UP;

			auto info = std::make_unique<GhostPenguinInfo>();
			info->modelRender.Init(GHOST_MODEL_PATH);

			// 浮上アニメーションの初期化と再生。
			info->floatCurve.Initialize(0.0f, GHOST_POSITION_OFFSET_Y, GHOST_SURFACING_ANIM_DURATION, util::EasingType::Linear, util::LoopMode::Once);
			info->floatCurve.Play();
			info->modelRender.SetTRS(dethPos, dethRot, dethScale);
			info->modelRender.Update();

			// 構造体のメンバにターゲットを設定。
			info->target = target;

			// ゴーストペンギンをリストに追加。
			m_ghostPenguins.push_back(std::move(info));
			// リストをキャッシュして同じ型に変換。
			m_ghostPenguinNum = static_cast<uint8_t>(m_ghostPenguins.size());
		}


		void ChildPenguinManager::UpdateGhostPenguins()
		{
			float deltaTime = g_gameTime->GetFrameDeltaTime();

			for (auto& info : m_ghostPenguins)
			{
				// 浮上アニメショーンの更新。
				if (info->floatCurve.IsPlaying())
				{
					info->floatCurve.Update(deltaTime);
					const Vector3 currentPosition = info->modelRender.GetPosition();
					const Vector3 translate = Vector3::Up * info->floatCurve.GetCurrentValue();
					const Vector3 nextPosition = currentPosition + translate;
					info->modelRender.SetPosition(nextPosition);
					info->modelRender.Update();

					continue;
				}
				
				// これ以降はアニメーション終了時の処理。
				info->timer += deltaTime;

				// 2秒待機後に非表示にする。
				if (info->timer >= GHOST_HIDDEN_WAIT_TIME && !info->isHidden)
				{
					info->isHidden = true;
					// nullチェック。
					if (info->target != nullptr)
					{
						// シロクマのステートマシーンを取得。
						EnemyStateMachine* sm = info->target->GetEnemyStateMachine();
						
						// 寝ている状態かつすでに帰巣中でなければデバフを発動!
						if (!sm->IsCoolDown() && !sm->IsReturnHome())
						{
							// シロクマを家に帰す。
							sm->SetReturnHome(true);
							// デバフを有効化。
							info->isDebuffActive = true;
							// タイマーをリセット。
							info->timer = 0.0f;
						}
					}
				}

				// デバフが有効でなければ、次の幽霊ペンギンへ。
				if (!info->isDebuffActive) continue;

				if (info->timer < GHOST_TOTAL_TIME)
				{
					if (!info->target) continue;

					// 幽霊ペンギンが消えた位置から、シロクマへのベクトルを計算する。
					Vector3 bearPos = info->target->GetEnemyStateMachine()->GetPosition();
					info->position = bearPos - info->modelRender.GetPosition();

					// ベクトルの長さが0に限りなく近い場合は正規化しない。
					if (info->position.LengthSq() > GHOST_DIR_NORMALIZE_SQ)
					{
						info->position.Normalize();
					}
				}
				// 時間経過でデバフ終了。
				else 
				{
					info->isDebuffActive = false;
				}
			}
		}


		void ChildPenguinManager::RenderGhostPenguins(RenderContext& rc)
		{
			for (auto& it : m_ghostPenguins)
			{
				if (!it->isHidden)
				{
					it->modelRender.Draw(rc);
				}
			}
		}
	}
}