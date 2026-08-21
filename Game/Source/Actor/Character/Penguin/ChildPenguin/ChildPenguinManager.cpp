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
#include "Source/Actor/Character/Penguin/Formation/Ult/UltContext.h"
#if defined(_DEBUG) || defined(K2_DEBUG)
#include "Source/Actor/Character/Penguin/Formation/FormationDebugMonitor.h"
#endif
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Manager/BattleManager.h"
#include "Source/Manager/FeverTimeManager.h"
#include "Source/Manager/IglooManager.h"
#include "Source/Manager/InGameUIManager.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/CPReaction/CPReactionTypes.h"
#include "Source/UI/RemainingChild/RemainingChildMenu.h"
#include <random>


namespace app
{
	namespace actor
	{
		namespace
		{
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
			/** 消える直前のフェードアウト時間(GHOST_HIDDEN_WAIT_TIME以下にすること) */
			constexpr float GHOST_FADE_OUT_DURATION = 1.0f;
			/** スケールアップ */
			const Vector3 GHOST_SCALE_UP = Vector3(0.8f, 1.0f, 1.0f);
			/** 方向正規化の平方 */
			constexpr float GHOST_DIR_NORMALIZE_SQ = 0.0001f;

			/** 陣形選択（スワイプ）SEの音量倍率 */
			constexpr float FORMATION_SWIPE_SE_VOLUME = 1.0f;

			/** ゴーストペンギン出現音の音量倍率 */
			constexpr float SPAWN_GHOST_PENGUIN_SE_VOLUME = 1.0f;


			//============================================//
			// 親の音が届く距離（子ペンギンの察知に使う）
			//
			// NoiseManager::GetDefaultParameter() が子ペンギンの足音・スライド音に
			// 使っている range と同じ数値にしてある。値を変えるときは
			// docs/子ペンギンの察知モデル.md の表も更新すること。
			//============================================//

			/** 歩き（NoiseManager の Sneak と同値） */
			constexpr float DADDY_NOISE_RADIUS_SNEAK = 200.0f;
			/** 走り（NoiseManager の Dash と同値） */
			constexpr float DADDY_NOISE_RADIUS_RUN = 400.0f;
			/** 滑り（NoiseManager の Slide と同値） */
			constexpr float DADDY_NOISE_RADIUS_SLIDE = 500.0f;
			/** ジャンプ（着地音。NoiseManager の Fall より控えめにしてある） */
			constexpr float DADDY_NOISE_RADIUS_JUMP = 300.0f;
			/** 泳ぎ（水音。歩きより少し広い程度） */
			constexpr float DADDY_NOISE_RADIUS_SWIM = 250.0f;


			//============================================//
			// 再集合の呼びかけ（Yボタン）
			//
			// 逃走をシロクマ最優先にした代わりに、散った群れを集め直す手段として置いた。
			// 「散開の距離と復帰の速さ」を調整する主なつまみはここと
			// ChildPenguinAIController.cpp の FLEE_* 定数。
			//============================================//

			/** 呼びかけの効果が続く時間（秒） */
			constexpr float REGROUP_CALL_DURATION = 2.0f;
			/**
			 * @brief 呼びかけのクールダウン（秒）
			 * @details 連打できると散開に代償が無くなり、
			 *          「群れごと失って集め直す」体験が生まれない。
			 */
			constexpr float REGROUP_CALL_COOLDOWN = 8.0f;
			/**
			 * @brief 呼びかけが届く距離
			 * @details 親の走りの音（400）より広く、シロクマの索敵距離（600）と同じ。
			 *          クマに追われて散った子が届く範囲に収まる想定。
			 */
			constexpr float REGROUP_CALL_RADIUS = 600.0f;

			/** 青い火の玉オフセット */
			const Vector3 BLUR_FIRE_BALL_OFFSET = Vector3(0.0f, 30.0f, 0.0f);
			/** 青い火の玉の初期回転 */
			const Quaternion BLUR_FIRE_BALL_ROTATION = Quaternion::Identity;
			/** 青い火の玉の初期スケール */
			const Vector3 BLUR_FIRE_BALL_SCALE_UP = Vector3(10.0f, 10.0f, 10.0f);
		}


		ChildPenguinManager* ChildPenguinManager::m_instance = nullptr;


		ChildPenguinManager::ChildPenguinManager()
			: m_ghostPenguinNum(0)
			, m_isGhostHidden(false)
			, m_ghostTimer(0.0f)
			, m_target(nullptr)
		{
#if defined(_DEBUG) || defined(K2_DEBUG)
			m_formationDebugMonitor = std::make_unique<FormationDebugMonitor>(this);
#endif
		}


		ChildPenguinManager::~ChildPenguinManager()
		{
			g_renderingEngine->UnregisterCustomRenderer(&m_rangeVisualizer);
		}


		void ChildPenguinManager::Start()
		{
			m_rangeVisualizer.Init();
			g_renderingEngine->RegisterCustomRenderer(&m_rangeVisualizer);

			// 陣形レベルアップ時にBattleManager経由でUIへ通知する
			m_formationController.SetOnLevelUp(
				[](const int level)
				{
					BattleManager::GetInstance().NotifyFormationLevelUp(level);
				}
			);

			/** 各子ペンギンのStartを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->StartWrapper();
			}
		}


		void ChildPenguinManager::Update()
		{
			/** 親の察知・逃走に使う共有データを、子のUpdateより先に1回だけ更新する */
			m_perceptionFrame++;
			UpdateDaddyNoiseRadius();
			UpdateBearThreats();
			UpdateRegroupCall();

			/** 各子ペンギンのUpdateを呼び出す */
			for (auto& cp : m_childPenguinList) {
				if (!cp) continue;
				cp->UpdateWrapper();
			}

			UpdateGhostPenguins();

			/** ウルト更新 */
			if (m_daddyPenguin != nullptr)
			{
				UltContext ctx{ this, m_daddyPenguin };
				m_formationController.UpdateUlt(g_gameTime->GetDeltaTime(), ctx);
			}

			/** 陣形切り替え演出（スライドUI）のロックタイマー更新 */
			m_formationController.UpdateSwitchLock(g_gameTime->GetFrameDeltaTime());

			/** L1/R1 で陣形を循環切り替え（スライド演出中は入力を無視する） */
			const bool isSwitching = m_formationController.IsSwitchingFormation();
			const bool isUltActive = m_formationController.IsUltActive();

			if (!isSwitching && !isUltActive)
			{
				if (g_pad[0]->IsTrigger(enButtonRB1))
				{
					const int next = (static_cast<int>(m_formationController.GetCurrentType()) + 1) % static_cast<int>(EnFormationType::Num);
					m_formationController.SwitchFormation(static_cast<EnFormationType>(next));
					m_formationController.StartSwitchTransition();
					// 次の陣形へ（右方向）の選択SEを鳴らす
					SoundManager::Get().PlaySE(enSoundKind_UltSwipeRight, FORMATION_SWIPE_SE_VOLUME);
				}
				else if (g_pad[0]->IsTrigger(enButtonLB1))
				{
					const int prev = (static_cast<int>(m_formationController.GetCurrentType()) + static_cast<int>(EnFormationType::Num) - 1) % static_cast<int>(EnFormationType::Num);
					m_formationController.SwitchFormation(static_cast<EnFormationType>(prev));
					m_formationController.StartSwitchTransition();
					// 前の陣形へ（左方向）の選択SEを鳴らす
					SoundManager::Get().PlaySE(enSoundKind_UltSwipeLeft, FORMATION_SWIPE_SE_VOLUME);
				}
			}

			/** LB2/RB2 でウルト発動 */
			if (g_pad[0]->IsTrigger(enButtonLB2) || g_pad[0]->IsTrigger(enButtonRB2))
			{
				ActivateUlt();
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
						m_daddyPenguin->GetTransform().m_position, forward, m_formationPositions, 1, 0);
				}
			}

			/** 陣形範囲ビジュアライザーの更新 */
			m_rangeVisualizer.SetVisible(true);
			if (m_daddyPenguin != nullptr)
			{
				const Vector3 center = m_daddyPenguin->GetTransform().m_position;
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
			float spawnRadius,
			float groundRayStartY
		)
		{
			// フィーバータイムの比率抽選・ランダム配置に再利用するためキャッシュしておく
			m_seriousNum = seriousNum;
			m_clingyNum = clingyNum;
			m_naughtyNum = naughtyNum;
			m_clumsyNum = clumsyNum;
			m_caringNum = caringNum;
			m_spawnRadius = spawnRadius;
			m_groundRayStartY = groundRayStartY;

			// フィーバー時のタイプ抽選器を一度だけ構築しておく（毎回再構築しない）
			m_typeDist = std::discrete_distribution<int>{
				static_cast<double>(m_seriousNum),
				static_cast<double>(m_clingyNum),
				static_cast<double>(m_naughtyNum),
				static_cast<double>(m_clumsyNum),
				static_cast<double>(m_caringNum)
			};

			// ==========================================
			// 生成するすべてのペンギンの「タイプ」を1つのリスト（プール）にまとめる
			// ==========================================
			std::vector<EnChildPenguinType> spawnPool;
			// メモリ確保の最適化（全匹分のサイズをあらかじめ確保）
			spawnPool.reserve(seriousNum + clingyNum + naughtyNum + clumsyNum + caringNum);

			// リストに指定された数だけタイプを追加していく
			for (int i = 0; i < seriousNum; i++) spawnPool.push_back(EnChildPenguinType::Serious);
			for (int i = 0; i < clingyNum; i++) spawnPool.push_back(EnChildPenguinType::Clingy);
			for (int i = 0; i < naughtyNum; i++) spawnPool.push_back(EnChildPenguinType::Naughty);
			for (int i = 0; i < clumsyNum; i++) spawnPool.push_back(EnChildPenguinType::Clumsy);
			for (int i = 0; i < caringNum; i++) spawnPool.push_back(EnChildPenguinType::Caring);

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

					// 次に作る群れのサイズを 1〜5 匹の間でランダムに決定する
					currentMaxClusterSize = clusterSizeDist(engine);
				}

				// 決まった群れの中心を渡して1匹生成
				SpawnOne(type, spawnRadius, currentClusterCenter, CLUSTER_RADIUS);

				currentClusterCount++;
			}
		}


		ChildPenguin* ChildPenguinManager::PlaceChildPenguin(EnChildPenguinType type, const Vector3& spawnPos)
		{
			CreateChildPenguin();
			auto* child = m_childPenguinList.back();
			child->SetLogId(m_nextLogId++);
			child->SetChildPenguinType(type);
			child->SetPosition(spawnPos);
			child->GetStateMachine()->SetPosition(spawnPos);
			child->StartWrapper();
			if (auto* lm = GameLogManager::GetInstance())
				// キー名を "type" にすると RecordSpawn 側がレコード種別で上書きしてしまうため、別名で渡す
				lm->RecordSpawn("penguin", child->GetLogId(), { {"penguin_type", child->GetChildPenguinTypeStr()} });
			return child;
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

			PlaceChildPenguin(type, spawnPos);
		}


		void ChildPenguinManager::SpawnFromSky(float dropHeight)
		{
			/** ステージ設定と同じ比率であらかじめ構築済みの抽選器からタイプを選ぶ */
			static std::mt19937 engine(std::random_device{}());
			const EnChildPenguinType type = static_cast<EnChildPenguinType>(m_typeDist(engine));

			/** ステージ全体からランダムなXZ座標を選び、地面より上空の高さから配置する */
			const Vector3 xzPos = GenerateRandomSpawnPosition(m_spawnRadius);
			const float groundY = GetGroundY(xzPos.x, xzPos.z);
			const Vector3 spawnPos = Vector3(xzPos.x, groundY + dropHeight, xzPos.z);

			PlaceChildPenguin(type, spawnPos);

			/** ステージ上のペンギン総数を1匹分増やす */
			ScoreManager::GetInstance().AddTotalCount();
		}


		Vector3 ChildPenguinManager::GenerateRandomSpawnPosition(float radius)
		{
			/** 拒絶サンプリング：円の外側に落ちた点を棄却して再抽選する */
			for (int i = 0; i < SPAWN_MAX_RETRY; i++)
			{
				const float x = util::RandomDevice::Random(-radius, radius);;
				const float z = util::RandomDevice::Random(-radius, radius);

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
			for (int i = 0; i < SPAWN_MAX_RETRY; i++)
			{
				const float x = util::RandomDevice::Random(-clusterRadius, clusterRadius);
				const float z = util::RandomDevice::Random(-clusterRadius, clusterRadius);

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
			const Vector3 rayStart = Vector3(x, m_groundRayStartY, z);
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

			/** 渦潮側が保持している target ポインタも無効化しておく（解放後の参照を防ぐ） */
			if (auto* wpMng = nature::WhirlpoolManager::GetInstance())
			{
				wpMng->NotifyPenguinDestroyed(penguin);
			}

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
				BattleManager::GetInstance().NotifyCPReactionChanged(penguin, ui::EnCPReactionType::Happy);

				if (auto* menu = InGameUIManager::GetInstance()->GetRemainingChildMenu())
				{
					menu->SetTarget(penguin);
				}

				if (auto* lm = GameLogManager::GetInstance())
					lm->QueueEvent({ {"ev", "penguin_join"}, {"penguin_id", penguin->GetLogId()} });

				/** フィーバータイム中は捕獲した分だけ投下キューに追加し、連続して降り続けるようにする */
				if (auto* feverManager = FeverTimeManager::GetInstance())
					feverManager->OnPenguinCaught();
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
				BattleManager::GetInstance().NotifyCPReactionChanged(penguin, ui::EnCPReactionType::Trouble);
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


		void ChildPenguinManager::ActivateUlt()
		{
			if (!m_daddyPenguin) return;
			UltContext ctx{ this, m_daddyPenguin };
			m_formationController.ActivateUlt(ctx);
		}


		void ChildPenguinManager::CalculateFormationPositions()
		{
			const Vector3 center = m_daddyPenguin->GetTransform().m_position;

			/** 親の向きから前方ベクトルを取得 */
			Vector3 forward = Vector3::Front;
			m_daddyPenguin->GetTransform().m_rotation.Apply(forward);

			const int actual = static_cast<int>(m_followers.size());

			/** count を actual+1 にすることで、現在のリングが満員になった瞬間に
			 *  m_outerRadius が次のリング半径へ切り替わり、入隊範囲円が先取り拡大する。
			 *  余分な1スロットは SortAndAssignFollowers では使われない。
			 *  レベル判定は actual で行い、速度ボーナス等への影響を正確に保つ。 */
			m_formationPositions.clear();
			m_formationController.CalculatePositions(
				center, forward, m_formationPositions, actual + 1, actual);
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


		void ChildPenguinManager::UpdateDaddyNoiseRadius()
		{
			m_daddyNoiseRadius = 0.0f;

			if (m_daddyPenguin == nullptr) return;

			auto* sm = m_daddyPenguin->GetStateMachine();
			if (sm == nullptr) return;

			/**
			 * 親が何をしているかで音の届く距離を決める。
			 * 数値は NoiseManager::GetDefaultParameter() が子ペンギンの
			 * 足音・スライド音に使っている range に合わせてある
			 * （Sneak 200 / Dash 400 / Slide 500）。
			 * 止まっている間は 0 ＝ 見つけてもらえない。
			 * これが「親が動いていないと子は寄ってこない」という手触りになり、
			 * Yボタンの再集合（DaddyPenguinController）が意味を持つ。
			 */
			if (sm->IsEqualCurrentState(PenguinSlidingState::ID())
				|| sm->IsEqualCurrentState(PenguinSlideStartState::ID())
				|| sm->IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				m_daddyNoiseRadius = DADDY_NOISE_RADIUS_SLIDE;
			}
			else if (sm->IsEqualCurrentState(PenguinRunState::ID()))
			{
				m_daddyNoiseRadius = DADDY_NOISE_RADIUS_RUN;
			}
			else if (sm->IsEqualCurrentState(PenguinJumpState::ID()))
			{
				m_daddyNoiseRadius = DADDY_NOISE_RADIUS_JUMP;
			}
			else if (sm->IsEqualCurrentState(PenguinSwimmingState::ID()))
			{
				m_daddyNoiseRadius = DADDY_NOISE_RADIUS_SWIM;
			}
			else if (sm->IsEqualCurrentState(PenguinSneakState::ID()))
			{
				m_daddyNoiseRadius = DADDY_NOISE_RADIUS_SNEAK;
			}
		}


		void ChildPenguinManager::UpdateBearThreats()
		{
			m_bearThreats.clear();

			auto* em = EnemyManager::GetInstance();
			if (em == nullptr) return;

			/**
			 * GetEnemies() と GetControllers() は同じ m_enemyList から同じ順で作られるので
			 * 添字が対応する。どちらも要素数はシロクマの数（最大9）なので、
			 * 毎フレーム1回ずつ作っても負荷にはならない（子ペンギンごとには呼ばない）
			 */
			const std::vector<Enemy*> enemies = em->GetEnemies();
			const std::vector<EnemyController*> controllers = em->GetControllers();

			const size_t count = min(enemies.size(), controllers.size());
			for (size_t i = 0; i < count; i++)
			{
				if (enemies[i] == nullptr || controllers[i] == nullptr) continue;

				/** 誰かを見つけて追っているクマだけを脅威として扱う */
				if (controllers[i]->GetFoundPenguin() == nullptr) continue;

				m_bearThreats.push_back(enemies[i]->GetTransform().m_position);
			}
		}


		bool ChildPenguinManager::FindNearestBearThreat(const Vector3& from, float radius, Vector3& outPos) const
		{
			const float radiusSq = radius * radius;
			float nearestSq = FLT_MAX;
			bool found = false;

			for (const Vector3& threatPos : m_bearThreats)
			{
				Vector3 diff = threatPos - from;
				diff.y = 0.0f;
				const float distSq = diff.LengthSq();

				if (distSq > radiusSq) continue;
				if (distSq >= nearestSq) continue;

				nearestSq = distSq;
				outPos = threatPos;
				found = true;
			}
			return found;
		}


		void ChildPenguinManager::CallRegroup()
		{
			/** クールダウン中は呼びかけられない */
			if (!CanCallRegroup()) return;

			m_regroupCallTimer = REGROUP_CALL_DURATION;
			m_regroupCallCooldown = REGROUP_CALL_COOLDOWN;

			if (auto* lm = GameLogManager::GetInstance())
			{
				lm->QueueEvent({ {"ev", "regroup_call"} });
			}
		}


		void ChildPenguinManager::UpdateRegroupCall()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			if (m_regroupCallTimer > 0.0f)
			{
				m_regroupCallTimer -= deltaTime;
			}
			if (m_regroupCallCooldown > 0.0f)
			{
				m_regroupCallCooldown -= deltaTime;
			}
		}


		float ChildPenguinManager::GetRegroupCallRadius() const
		{
			return REGROUP_CALL_RADIUS;
		}


		float ChildPenguinManager::GetRegroupCallCooldownRatio() const
		{
			if (m_regroupCallCooldown <= 0.0f) return 0.0f;
			return m_regroupCallCooldown / REGROUP_CALL_COOLDOWN;
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
			, alphaCurve()
			, target(nullptr)
			, timer(0.0f)
			, isHidden(false)
			, isFadingOut(false)
			, isDebuffActive(false)
			, position(Vector3::Zero)
			, handle(INVALID_EFFECT_HANDLE)
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

			// 透明→実体化のフェードインを浮上アニメーションと同じ時間で再生。
			info->modelRender.SetAlpha(0.0f);
			info->alphaCurve.Initialize(0.0f, 1.0f, GHOST_SURFACING_ANIM_DURATION, util::EasingType::Linear, util::LoopMode::Once);
			info->alphaCurve.Play();

			// 青い火の玉エフェクトを再生。
			info->handle = EffectManager::Get().PlayEffect(
				EnEffectKind::GhostPenguinBlurFireBall,
				info->modelRender.GetPosition(),
				Quaternion::Identity,
				BLUR_FIRE_BALL_SCALE_UP
			);

			// エフェクト追従。
			EffectManager::Get().AttachEffect(
				info->handle,
				&info->modelRender.GetPosition(),
				BLUR_FIRE_BALL_OFFSET
			);

			//if(info->handle == INVALID_EFFECT_HANDLE)
			//{
			//}

			// ゴーストペンギンの出現音を再生。
			SoundManager::Get().PlaySE(enSoundKind_GhostPenguinReaction, SPAWN_GHOST_PENGUIN_SE_VOLUME, enSoundPriority_Hight);

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

					// 浮上と同時に透明→実体化のフェードインを進める。
					info->alphaCurve.Update(deltaTime);
					info->modelRender.SetAlpha(info->alphaCurve.GetCurrentValue());

					continue;
				}

				// これ以降はアニメーション終了時の処理。
				info->timer += deltaTime;

				// 非表示になる直前から実体化→透明のフェードアウトを開始する。
				if (!info->isFadingOut && !info->isHidden &&
					info->timer >= GHOST_HIDDEN_WAIT_TIME - GHOST_FADE_OUT_DURATION)
				{
					info->isFadingOut = true;
					info->alphaCurve.Initialize(1.0f, 0.0f, GHOST_FADE_OUT_DURATION, util::EasingType::Linear, util::LoopMode::Once);
					info->alphaCurve.Play();
				}

				// フェードアウト中は透明度を更新する。
				if (info->isFadingOut && info->alphaCurve.IsPlaying())
				{
					info->alphaCurve.Update(deltaTime);
					info->modelRender.SetAlpha(info->alphaCurve.GetCurrentValue());
				}

				// 2秒待機後に非表示にする。
				if (info->timer >= GHOST_HIDDEN_WAIT_TIME && !info->isHidden)
				{
					info->isHidden = true;
					info->modelRender.SetAlpha(0.0f);

					// 青い火の玉エフェクトを停止。
					if (info->handle != INVALID_EFFECT_HANDLE)
					{
						EffectManager::Get().StopEffect(info->handle);
						info->handle = INVALID_EFFECT_HANDLE;
					}

					// nullチェック。
					if (info->target != nullptr)
					{
						// シロクマのステートマシーンを取得。
						EnemyStateMachine* sm = info->target->GetEnemyStateMachine();

						// デバフを発動!
						if (!sm->IsCoolDown() && !sm->IsReturnHome())
						{
							// デバフが発動したことをシロクマに通知。
							sm->SetDebuffReturnHome(true);
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