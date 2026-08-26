/**
 * @file EnemyController.cpp
 * @brief エネミーのコントローラー
 * @author 立山
 */
#include "stdafx.h"
#include <time.h>

#include "Enemy.h"
#include "EnemyController.h"
#include "EnemyManager.h"

#include "Source/Achivement/AchievementManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStatus.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/Manager/IglooManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 徘徊の到達判定距離（シロクマのradiusに合わせて拡大） */
			constexpr float ARRIVE_DIST_WANDERING = 120.0f;
			/** 帰巣の到達判定距離（この距離に入ったら巣の上にワープして睡眠へ移行） */
			constexpr float ARRIVE_DIST_RETURN_HOME = 120.0f;

			/** スタック検出：この距離以下しか動いていなければ停滞とみなす */
			constexpr float STUCK_MOVE_THRESHOLD = 5.0f;
			/** スタック検出：この時間（秒）停滞し続けたら強制スキップ */
			constexpr float STUCK_TIME_LIMIT = 3.0f;

			// --- マジックナンバー排除用の内部定数 ---
			/** ターゲットをIdleにする距離の二乗 */
			constexpr float DIST_TO_IDLE_SQ = 600.0f * 600.0f;
			/** 視野角（角度） */
			constexpr float VIEW_ANGLE = 70.0f;
			/** 巣から離れすぎたと判定する最大距離 */
			constexpr float MAX_DIST_FROM_HOME = 500.0f;
			/** ランダムな待機時間を計算するための係数（モジュロ用） */
			constexpr int RANDOM_IDLE_TIME_MOD = 500;
			/** ランダムな待機時間を計算するための係数（乗算用） */
			constexpr float RANDOM_IDLE_TIME_MULT = 0.01f;
			/** 索敵閾値 */
			constexpr float SEARCH_THRESHOLD = 15.0f;
			/** 音の目標地点までの到着判定距離の二乗 */
			constexpr float REACHED_TARGET_DIST_SQ = 5.0f * 5.0f;
			/** 音が鳴らなくなってから諦めるまでの時間 */
			constexpr float GIVE_UP_SEARCH_TIME = 3.0f;
			/** 方向ベクトルのゼロ除算防止用のしきい値 */
			constexpr float CHASE_DIR_NORMALIZE_SQ = 0.001f;
			/** 通常の攻撃発生距離 */
			constexpr float NORMAL_ATTACK_DIST = 80.0f;
			/** かまくらへの攻撃発生距離 */
			constexpr float IGLOO_ATTACK_DIST = 240.0f;
			/** 最後に見失った位置に到達したと判定する距離 */
			constexpr float ARRIVE_LAST_KNOWN_POS_DIST = 20.0f;
			/** 帰巣時に移動を停止する距離の二乗 */
			constexpr float RETURN_HOME_STOP_DIST_SQ = 200.0f;

			/**
			 * @brief 索敵（Search）中の旋回速度の倍率
			 * @details 音に気づいたシロクマが「？」を出しながらゆっくり振り向くための値。
			 *          視界判定は体の正面（transform）基準なので、振り向きが遅いぶんだけ
			 *          プレイヤーに逃げる猶予が生まれる（2026-08-23 試遊フィードバック）。
			 */
			constexpr float SEARCH_TURN_MULTIPLIER = 0.25f;

			/**
			 * @brief 咆哮のラジアルブラーが届く「プレイヤーとの距離」
			 * @details この距離より遠いクマの咆哮では画面演出を出さない。
			 *          遠くの無関係な咆哮で画面がボケると理不尽なため。
			 */
			constexpr float ROAR_BLUR_MAX_DISTANCE = 900.0f;
			/** 咆哮のラジアルブラーの継続時間（秒） */
			constexpr float ROAR_BLUR_DURATION = 1.0f;
			/**
			 * @brief 咆哮のラジアルブラーの立ち上がり時間（秒）
			 * @details クマが吠えるまでの溜めに合わせて加速的に強くなり、
			 *          ここでピークに達した後、残り時間で線形に減衰する。
			 */
			constexpr float ROAR_BLUR_ATTACK_TIME = 0.3f;
			/** 咆哮のラジアルブラーの最小強度（距離境界ぎりぎりでの強さ） */
			constexpr float ROAR_BLUR_MIN_STRENGTH = 0.3f;

			//============================================//
			// 密集陣ウルトの攻撃無効化＋反撃
			//============================================//

			/** 無効化時にクマを弾き飛ばす距離 */
			constexpr float NULLIFY_KNOCKBACK_DISTANCE = 150.0f;
			/** 無効化時のヒットストップの時間倍率と長さ（実時間・秒） */
			constexpr float NULLIFY_HITSTOP_SCALE = 0.4f;
			constexpr float NULLIFY_HITSTOP_DURATION = 0.15f;
			/** 無効化時の画面揺れ（振幅・秒）
			 *  2026-08-25 酔い対策で全シェイクを一段減らした（揺れ自体も滑らかなノイズ方式に変更済み） */
			constexpr float NULLIFY_SHAKE_STRENGTH = 8.0f;
			constexpr float NULLIFY_SHAKE_DURATION = 0.25f;

			/** 咆哮の画面揺れ（至近時の最大振幅・秒）。咆哮はラジアルブラーも重なるため特に控えめに */
			constexpr float ROAR_SHAKE_STRENGTH = 6.0f;
			constexpr float ROAR_SHAKE_DURATION = 0.4f;

			/** かまくら崩壊の画面揺れ（振幅・秒） */
			constexpr float IGLOO_BREAK_SHAKE_STRENGTH = 10.0f;
			constexpr float IGLOO_BREAK_SHAKE_DURATION = 0.4f;
		}


		// static変数の初期化
		std::map<EnemyController::EnEnemyStateID, EnemyController::AIState> EnemyController::m_stateMap;


		EnemyController::EnemyController()
			: m_target(nullptr)
			, m_foundPenguin(nullptr)
			, m_elapsedTime(0.0f)
			, m_prePosition(Vector3::Zero)
			, m_startPosition(Vector3::Zero)
			, m_lastKnownPenguinPos(Vector3::Zero)
			, m_isStun(false)
			, m_isHomeInitialized(false)
			, m_coolDownTimer(0.0f)
			, m_maxEatCount(3)
			, m_eatCount(0)
			, m_isFull(false)
			, m_isParamInitialized(false)
			, m_attackDuration(1.5f)
			, m_attackTimer(0.0f)
			, m_isAttacking(false)
			, m_searchTimer(0.0f)
			, m_lastCheckPosition(Vector3::Zero)
			, m_stuckTimer(0.0f)
		{
			static bool ini = false;
			if (!ini)
			{
				Initialize();
				ini = true;
			}
		}


		EnemyController::~EnemyController()
		{}


		bool EnemyController::Start()
		{
			return true;
		}


		void EnemyController::Update()
		{
			if (!m_isParamInitialized)
			{
				m_maxEatCount = m_target->GetEnemyStateMachine()->GetOwnerStatus()->GetMaxEat();

				m_isParamInitialized = true;
			}

			auto* currentState = FindAIState(m_currentState);
			if (currentState == nullptr) {
				K2_ASSERT(false, "対象の処理が見つかりません\n");
				return;
			}

			/** 初回起動時のEnter処理 */
			if (!m_isInitialized) {
				currentState->enter(this);
				m_isInitialized = true;
			}

			/** 遷移判定 */
			const int nextState = currentState->check(this);
			if (nextState != -1 && nextState != m_currentState) {
				// C言語スタイルキャストを static_cast に修正
				ChangeState(static_cast<EnEnemyStateID>(nextState));
				currentState = FindAIState(m_currentState);
			}

			if (m_isStun)
			{
				m_target->GetEnemyStateMachine()->SetStun(true);
				m_isStun = false;
			}

			/** 現在のステートのアップデート */
			currentState->update(this);
			if (m_currentState == enEnemyState_Search ||
				m_currentState == enEnemyState_Wandering ||
				m_currentState == enEnemyState_Chase ||
				m_currentState == enEnemyState_Swim)
			{
				ChildPenguin* found = FindTarget();
				if (found != nullptr)
				{
					m_foundPenguin = found;
					m_lastKnownPenguinPos = found->GetTransform().m_position;
				}
			}
		}


		void EnemyController::Render(RenderContext& renderContext)
		{}


		void EnemyController::SetTarget(Enemy* target)
		{
			m_target = target;
		}


		void EnemyController::AddTargetPos(const Vector3& pos)
		{
			m_wanderingPosList.push_back(pos);
		}


		void EnemyController::ChangeState(EnEnemyStateID nextState)
		{
			/** 指定したnextStateがおかしい */
			if (nextState < enEnemyState_Invalid || nextState >= enEnemyState_Num) {
				return;
			}

			auto* currentState = FindAIState(m_currentState);
			/** 現在のステートのExit処理 */
			currentState->exit(this);
			/** ステートの更新 */
			m_currentState = nextState;
			/** 新しいステートのEnterを呼ぶ */
			currentState = FindAIState(m_currentState);
			currentState->enter(this);
		}


		ChildPenguin* EnemyController::FindTarget()
		{
			auto penguinList = actor::ChildPenguinManager::GetInstance()->GetChildPenguin();
			for (auto* penguin : penguinList)
			{
				Vector3 targetPos = m_target->GetTransform().m_position;
				Vector3 diff = penguin->GetTransform().m_position - targetPos;
				diff.y = 0.0f;
				if (diff.LengthSq() > DIST_TO_IDLE_SQ)
				{
					continue;
				}

				diff.Normalize();

				// 視界の向きは移動方向ではなく体の正面（transform）を使う。
				// 移動方向は入力の瞬間に切り替わるため、音の方向へ「振り向いた瞬間」に
				// 視界も一瞬で回ってしまい、旋回をゆっくりにした意味が無くなる
				Vector3 forward = Vector3::Front;
				m_target->GetTransform().m_rotation.Apply(forward);
				forward.y = 0.0f;
				if (forward.LengthSq() <= FLT_EPSILON)
				{
					continue;
				}
				forward.Normalize();

				float cosv = forward.Dot(diff);
				float cosAngle = cosf(Math::PI / 180.0f * VIEW_ANGLE);
				if (cosv < cosAngle)
				{
					continue;
				}

				nsBeastEngine::nsCollision::RaycastHit hit;

				if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(
					targetPos,
					penguin->GetTransform().m_position,
					hit,
					nsBeastEngine::nsCollision::ALL_COLLISION_ATTRIBUTE_MASK))
				{
					if (hit.colObject)
					{
						int attr = hit.colObject->getUserIndex();

						if (attr & nsBeastEngine::nsCollision::CollisionAttribute::Ground)
						{
							continue;
						}
					}
				}
				return penguin;

			}
			return nullptr;
		}


		bool EnemyController::IsFarFromHome() const
		{
			Vector3 pos = m_target->GetTransform().m_position;
			Vector3 toHome = m_target->GetHomePosition() - pos;

			if (toHome.LengthSq() > MAX_DIST_FROM_HOME * MAX_DIST_FROM_HOME)
			{
				return true;
			}
			return false;
		}


		void EnemyController::Initialize()
		{
			/** 待機 */
			RegisterState(enEnemyState_Idle, EnterIdle, UpdateIdle, ExitIdle, CheckIdle);
			/** サーチ */
			RegisterState(enEnemyState_Search, EnterSearch, UpdateSearch, ExitSearch, CheckSearch);
			/** 徘徊 */
			RegisterState(enEnemyState_Wandering, EnterWandering, UpdateWandering, ExitWandering, CheckWandering);
			/** チェイス */
			RegisterState(enEnemyState_Chase, EnterChase, UpdateChase, ExitChase, CheckChase);
			/** ジャンプ */
			RegisterState(enEnemyState_Jump, EnterJump, UpdateJump, ExitJump, CheckJump);
			/** 泳ぐ */
			RegisterState(enEnemyState_Swim, EnterSwim, UpdateSwim, ExitSwim, CheckSwim);
			/** 攻撃 */
			RegisterState(enEnemyState_Attack, EnterAttack, UpdateAttack, ExitAttack, CheckAttack);
			/** 帰巣 */
			RegisterState(enEnemyState_ReturnHome, EnterReturnHome, UpdateReturnHome, ExitReturnHome, CheckReturnHome);
			/** クールダウン */
			RegisterState(enEnemyState_CoolDown, EnterCoolDown, UpdateCoolDown, ExitCoolDown, CheckCoolDown);
			/** 咆哮 */
			RegisterState(enEnemyState_Roar, EnterRoar, UpdateRoar, ExitRoar, CheckRoar);
		}


		/** 各ステートの処理はこの下に書いていく */


		/** 待機 */
		void EnemyController::EnterIdle(EnemyController* enemy)
		{
			enemy->m_elapsedTime = 0.0f;

			auto* sm = enemy->m_target->GetEnemyStateMachine();

			// 完全停止
			sm->SetStickLAmount(0.0f);

			enemy->m_foundPenguin = nullptr;
		}


		void EnemyController::UpdateIdle(EnemyController* enemy)
		{
			enemy->m_elapsedTime += g_gameTime->GetFrameDeltaTime();
		}


		void EnemyController::ExitIdle(EnemyController* enemy)
		{}


		int EnemyController::CheckIdle(EnemyController* enemy)
		{
			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			// 音で索敵フラグが立ったらSearchへ
			if (enemy->m_target->GetEnemyStateMachine()->IsSearch())
			{
				return enEnemyState_Search;
			}

			const float idleTime = static_cast<float>(rand() % RANDOM_IDLE_TIME_MOD) * RANDOM_IDLE_TIME_MULT;
			if (enemy->m_elapsedTime > idleTime) {
				// 時間経過によるランダム遷移は、(0,0,0)へ行かないようWanderingに統一
				return enEnemyState_Wandering;
			}
			return enEnemyState_Invalid;
		}


		/** スタン */
		void EnemyController::EnterStun(EnemyController* enemy)
		{}


		void EnemyController::UpdateStun(EnemyController* enemy)
		{}


		void EnemyController::ExitStun(EnemyController* enemy)
		{}


		int EnemyController::CheckStun(EnemyController* enemy)
		{
			return enEnemyState_Invalid;
		}


		/** サーチ（音のした場所へ向かう） */
		void EnemyController::EnterSearch(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			enemy->m_target->GetEnemyStateMachine()->SetSearch(true);
			enemy->m_searchTimer = 0.0f; // タイマーリセット

			// 音に気づいた合図の「？」を出し、ゆっくり振り向かせる。
			// 「音で気づく（？）→ 振り向く → 視界に入る（！）→ 追いかける」の段階を見せるため
			enemy->m_target->GetEnemyStateMachine()->SetTurnSpeedMultiplier(SEARCH_TURN_MULTIPLIER);
			BattleManager::GetInstance().NotifyEnemyReactionChanged(
				enemy->m_target, ui::EnCPReactionType::Question);
		}


		void EnemyController::UpdateSearch(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			// 1. 視界によるペンギン発見処理
			enemy->m_foundPenguin = enemy->FindTarget();
			if (enemy->m_foundPenguin != nullptr)
			{
				enemy->m_target->GetEnemyStateMachine()->SetFindPenguin(true);
				return;
			}

			// 2. 索敵中も耳を澄ませて、新しい音がしたら目標地点を更新する
			Vector3 loudestPos;
			float totalNoise = app::NoiseManager::GetInstance().CalculateTotalNoiseAt(enemy->m_target->GetTransform().m_position, loudestPos);

			auto* sm = enemy->m_target->GetEnemyStateMachine();

			if (totalNoise >= SEARCH_THRESHOLD)
			{
				sm->SetSearchTargetPos(loudestPos);
				enemy->m_searchTimer = 0.0f; // 新しい音がしたので諦めタイマーをリセット
			}

			// 3. 音がした目標地点へ歩く
			Vector3 targetPos = sm->GetSearchTargetPos();
			Vector3 currentPos = enemy->m_target->GetTransform().m_position;
			Vector3 toTarget = targetPos - currentPos;
			toTarget.y = 0.0f; // 高さは無視して平面の距離だけを見る

			float distanceSq = toTarget.LengthSq();
			if (distanceSq > REACHED_TARGET_DIST_SQ)
			{
				toTarget.Normalize();
				sm->SetMoveDirection(toTarget);
				sm->SetStickLAmount(1.0f); // スティックを倒して歩かせる
			}
			else
			{
				// 目標地点に到着した
				sm->SetStickLAmount(0.0f); // 立ち止まる

				// 到着してしばらく（例：3秒）音が鳴らなければ諦める
				enemy->m_searchTimer += g_gameTime->GetFrameDeltaTime();
				if (enemy->m_searchTimer > GIVE_UP_SEARCH_TIME)
				{
					sm->SetSearch(false); // 諦めフラグを落とす
				}
			}
		}


		void EnemyController::ExitSearch(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
			enemy->m_target->GetEnemyStateMachine()->SetSearch(false);
			// 索敵用に落としていた旋回速度を戻す
			enemy->m_target->GetEnemyStateMachine()->SetTurnSpeedMultiplier(1.0f);
			enemy->m_searchTimer = 0.0f;
		}


		int EnemyController::CheckSearch(EnemyController* enemy)
		{
			// ターゲット見つけたらチェイス
			if (enemy->m_foundPenguin != nullptr)
			{
				return enEnemyState_Roar;
			}
			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			// 諦めフラグが立っていたら徘徊に戻る
			if (!enemy->m_target->GetEnemyStateMachine()->IsSearch()) {
				return enEnemyState_Wandering;
			}

			return enEnemyState_Invalid;
		}


		/** 徘徊 */
		void EnemyController::EnterWandering(EnemyController* enemy)
		{
			// スタック検出タイマーをリセット
			enemy->m_stuckTimer = 0.0f;
			enemy->m_lastCheckPosition = enemy->m_target->GetTransform().m_position;
		}


		void EnemyController::UpdateWandering(EnemyController* enemy)
		{
			// 対象座標までの距離
			Vector3 distance = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex] - enemy->m_target->GetTransform().m_position;
			// 方向
			Vector3 direction = distance;
			direction.Normalize();

			enemy->m_target->GetEnemyStateMachine()->SetMoveDirection(direction);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(1.0f);

			// スタック検出：前回チェック位置からほとんど動いていない場合はタイマーを進める
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			Vector3 currentPos = enemy->m_target->GetTransform().m_position;
			Vector3 moved = currentPos - enemy->m_lastCheckPosition;
			moved.y = 0.0f;

			if (moved.LengthSq() < STUCK_MOVE_THRESHOLD * STUCK_MOVE_THRESHOLD)
			{
				enemy->m_stuckTimer += deltaTime;
			}
			else
			{
				// 動いていればタイマーをリセット
				enemy->m_stuckTimer = 0.0f;
				enemy->m_lastCheckPosition = currentPos;
			}
		}


		void EnemyController::ExitWandering(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);

			enemy->m_wanderingPosListIndex++;
			if (enemy->m_wanderingPosListIndex >= enemy->m_wanderingPosList.size())
			{
				enemy->m_wanderingPosListIndex = 0;
			}
		}


		int EnemyController::CheckWandering(EnemyController* enemy)
		{
			if (enemy->m_foundPenguin != nullptr)
			{
				return enEnemyState_Roar;
			}

			if (enemy->m_target->GetEnemyStateMachine()->IsReturnHome())
			{
				return enEnemyState_ReturnHome;
			}

			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			// 耳で音を検知してフラグが立ったら、索敵ステートへ移行
			if (enemy->m_target->GetEnemyStateMachine()->IsSearch()) {
				return enEnemyState_Search;
			}

			Vector3 distance = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex] - enemy->m_target->GetTransform().m_position;

			// 到達判定（閾値拡大）
			if (distance.Length() <= ARRIVE_DIST_WANDERING)
			{
				return enEnemyState_Idle;
			}

			// スタック検出：一定時間動けていない場合は次のポイントへ強制スキップ
			if (enemy->m_stuckTimer >= STUCK_TIME_LIMIT)
			{
				return enEnemyState_Idle;
			}

			return enEnemyState_Invalid;
		}


		/** チェイス */
		void EnemyController::EnterChase(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(true);
			// Chase開始を StateMachine に通知する
			enemy->m_target->GetEnemyStateMachine()->SetIsChasing(true);

			// 追跡対象が隊列中の子ペンギンであれば HasChased フラグを立てる
			if (enemy->m_foundPenguin != nullptr)
			{
				if (app::actor::ChildPenguinManager::GetInstance()->IsFollower(enemy->m_foundPenguin))
				{
					enemy->m_hasChased = true;
				}
			}

			enemy->m_prePosition = enemy->m_target->GetTransform().m_position;
		}


		void EnemyController::UpdateChase(EnemyController* enemy)
		{
			Vector3 enemyPos = enemy->m_target->GetTransform().m_position;

			float movedDist = (enemyPos - enemy->m_prePosition).Length();
			enemy->m_prePosition = enemyPos;

			auto* status = enemy->m_target->GetEnemyStateMachine()->GetOwnerStatus();

			Vector3 targetPos;
			if (enemy->m_foundPenguin != nullptr)
			{
				targetPos = enemy->m_foundPenguin->GetTransform().m_position;
			}
			else
			{
				targetPos = enemy->m_lastKnownPenguinPos;
			}

			Vector3 dir = targetPos - enemyPos;
			dir.y = 0.0f;

			if (dir.LengthSq() > CHASE_DIR_NORMALIZE_SQ)
			{
				dir.Normalize();
			}

			auto* sm = enemy->m_target->GetEnemyStateMachine();
			sm->SetMoveDirection(dir);
			sm->SetStickLAmount(1.0f);

			auto* mutableStatus = sm->GetOwnerStatusMutable();
			if (mutableStatus != nullptr)
			{
				// フレーム時間 × 減衰率(staminaDrainRate) を計算して減らす
				float drainAmount = mutableStatus->GetStaminaDrainRate() * g_gameTime->GetFrameDeltaTime();
				mutableStatus->DecreaseStamina(drainAmount);
			}
		}


		void EnemyController::ExitChase(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(false);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
			// Chase終了を StateMachine に通知する
			enemy->m_target->GetEnemyStateMachine()->SetIsChasing(false);

			if (auto* am = app::achievement::AchievementManager::GetInstance()) {
				auto* baseAchieve = am->GetAchievement(Hash32("MaxEscapeMarking"));
				if (auto* recordAchieve = dynamic_cast<app::achievement::RecordAchievement*>(baseAchieve)) {
					// 現在連れている子ペンギンの数を取得して更新
					int currentFollowers = app::actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
					recordAchieve->UpdateRecord(static_cast<uint32_t>(currentFollowers));
				}
			}
		}


		int EnemyController::CheckChase(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();
			auto* status = sm->GetOwnerStatus();

			if (sm->IsReturnHome())
			{
				enemy->m_foundPenguin = nullptr;
				return enEnemyState_ReturnHome;
			}

			// スタミナが尽きたらターゲットを諦め、巣へ帰る
			if (status->IsStaminaEmpty())
			{
				enemy->m_foundPenguin = nullptr;
				return enEnemyState_ReturnHome;
			}

			if (enemy->m_foundPenguin != nullptr)
			{
				Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
				Vector3 penguinPos = enemy->m_foundPenguin->GetTransform().m_position;

				Vector3 diff = penguinPos - enemyPos;
				diff.y = 0.0f;

				float lostDist = status->GetLostChaseDistance();

				// 距離が離れすぎたらターゲットを諦め、巣へ帰る
				if (diff.LengthSq() > lostDist * lostDist)
				{
					enemy->m_foundPenguin = nullptr;
					return enEnemyState_ReturnHome;
				}
			}

			if (enemy->m_foundPenguin != nullptr)
			{
				Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
				Vector3 penguinPos = enemy->m_foundPenguin->GetTransform().m_position;

				Vector3 diff = penguinPos - enemyPos;
				diff.y = 0.0f;

				float distSq = diff.LengthSq();

				// 一定距離で攻撃
				float attackDist = NORMAL_ATTACK_DIST;

				// ターゲットがかまくらの中にいるかチェック
				const auto& insidePenguins = app::actor::IglooManager::GetInstance().GetInsidePenguins();
				if (std::find(insidePenguins.begin(), insidePenguins.end(), enemy->m_foundPenguin) != insidePenguins.end())
				{
					// ★ 重要：かまくらの上に登らないよう、壁の手前で攻撃モーションに入る距離
					attackDist = IGLOO_ATTACK_DIST;
				}

				if (distSq <= attackDist * attackDist)
				{
					return enEnemyState_Attack;
				}

				return enEnemyState_Invalid;
			}
			if (enemy->m_foundPenguin == nullptr)
			{
				Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
				Vector3 diff = enemy->m_lastKnownPenguinPos - enemyPos;
				diff.y = 0.0f;

				// 最後の位置に到達したらIdle
				if (diff.LengthSq() <= ARRIVE_LAST_KNOWN_POS_DIST * ARRIVE_LAST_KNOWN_POS_DIST)
				{
					return enEnemyState_Idle;
				}
			}
			return enEnemyState_Invalid;
		}


		/** ジャンプ */
		void EnemyController::EnterJump(EnemyController* enemy)
		{}


		void EnemyController::UpdateJump(EnemyController* enemy)
		{}


		void EnemyController::ExitJump(EnemyController* enemy)
		{}


		int EnemyController::CheckJump(EnemyController* enemy)
		{
			return enEnemyState_Invalid;
		}


		/** 泳ぐ */
		void EnemyController::EnterSwim(EnemyController* enemy)
		{}


		void EnemyController::UpdateSwim(EnemyController* enemy)
		{
			Vector3 targetPos;

			// 帰巣中なら巣へ、ターゲットがいるならペンギンへ、それ以外は徘徊ポイントへ
			if (enemy->m_target->GetEnemyStateMachine()->IsReturnHome())
			{
				targetPos = enemy->m_target->GetHomePosition();
			}
			else if (enemy->m_foundPenguin != nullptr)
			{
				targetPos = enemy->m_foundPenguin->GetTransform().m_position;
			}
			else
			{
				targetPos = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex];
			}

			// 対象座標までの距離
			Vector3 distance = targetPos - enemy->m_target->GetTransform().m_position;
			// 方向
			Vector3 direction = distance;
			direction.Normalize();

			enemy->m_target->GetEnemyStateMachine()->SetMoveDirection(direction);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitSwim(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);

			enemy->m_wanderingPosListIndex++;
			if (enemy->m_wanderingPosListIndex >= enemy->m_wanderingPosList.size())
			{
				enemy->m_wanderingPosListIndex = 0;
			}
		}


		int EnemyController::CheckSwim(EnemyController* enemy)
		{
			return enEnemyState_Invalid;
		}


		/** 攻撃 */
		void EnemyController::EnterAttack(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();

			// 足を止めて攻撃フラグを立てる
			sm->SetStickLAmount(0.0f);
			sm->SetMoveVector(Vector3::Zero);
			sm->SetActionButtonX(true);
			sm->SetIsNearPenguin(true);

			// =======================================================
			// 攻撃開始時点での状態を「記憶」する
			// =======================================================
			enemy->m_isTargetInsideIglooAtStart = false;
			enemy->m_targetIglooKeyAtStart = "";

			if (enemy->m_foundPenguin != nullptr)
			{
				// ターゲットがかまくらの中にいるかチェック
				const auto& insidePenguins = app::actor::IglooManager::GetInstance().GetInsidePenguins();
				if (std::find(insidePenguins.begin(), insidePenguins.end(), enemy->m_foundPenguin) != insidePenguins.end())
				{
					enemy->m_isTargetInsideIglooAtStart = true;

					// 壊すべきかまくらを特定して名前を覚えておく
					Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
					enemy->m_targetIglooKeyAtStart = StageSystem::GetInstance()->GetNearestIglooKey(enemyPos);
				}
			}
		}


		void EnemyController::UpdateAttack(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();

			// =======================================================
			// ★ IStateから「今叩きつけた！」という合図が来た瞬間だけ実行
			// =======================================================
			if (sm->IsAttackImpact())
			{
				// 二重実行防止のため、すぐに合図をリセット
				sm->SetAttackImpact(false);

				// 1. かまくらの破壊処理（開始時に「中にいる」と判定されていた場合）
				if (enemy->m_isTargetInsideIglooAtStart && !enemy->m_targetIglooKeyAtStart.empty())
				{
					Vector3 iglooPos = StageSystem::GetInstance()->GetObjectPosition(enemy->m_targetIglooKeyAtStart);

					// 実際に壊す
					StageSystem::GetInstance()->BreakIgloo(enemy->m_targetIglooKeyAtStart);

					// かまくら崩壊の衝撃を画面揺れで見せる
					if (auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
						camera::GameCamera::ID()))
					{
						gameCamera->StartShake(IGLOO_BREAK_SHAKE_STRENGTH, IGLOO_BREAK_SHAKE_DURATION);
					}
					IglooManager::GetInstance().EjectAllPenguins(iglooPos);
					if (auto* lm = GameLogManager::GetInstance())
						lm->QueueEvent({ {"ev", "igloo_broken"}, {"key", enemy->m_targetIglooKeyAtStart}, {"bear_id", enemy->m_target->GetLogId()} });
				}
				else
				{
					// ★修正：かまくらの外で「完全に叩きつけが成功した瞬間」に満腹度を増やす
					enemy->m_eatCount++;
					if (enemy->m_eatCount >= enemy->m_maxEatCount)
					{
						enemy->m_isFull = true;
					}
				}
			}
			// =======================================================
			// 2. 攻撃開始直後（1フレーム目）のペンギン足止め処理
			// =======================================================
			if (enemy->m_foundPenguin != nullptr)
			{
				// 密集陣ウルト中は、隊列の子への攻撃を無効化してクマへ反撃する。
				// 「守り切った」だけで終わらせず、弾き飛ばし＋スタン＋短いヒットストップで
				// プレイヤーに意趣返しの手応えを返す
				auto* cpManager = actor::ChildPenguinManager::GetInstance();
				if (BattleManager::GetInstance().IsBearAttackNullified()
					&& cpManager != nullptr
					&& cpManager->IsFollower(enemy->m_foundPenguin))
				{
					auto* bear = enemy->m_target;

					/** 攻撃した子と反対方向へ弾き飛ばす（キノマティック移動） */
					Vector3 away = bear->GetTransform().m_position
						- enemy->m_foundPenguin->GetTransform().m_position;
					away.y = 0.0f;
					if (away.LengthSq() > FLT_EPSILON) { away.Normalize(); }
					else { away = Vector3::Front; }

					Vector3 knockbackPos = bear->GetTransform().m_position
						+ away * NULLIFY_KNOCKBACK_DISTANCE;
					bear->GetCharacterController()->SetPosition(knockbackPos);
					bear->GetCharacterController()->RequestTeleport();
					bear->GetEnemyStateMachine()->SetPosition(knockbackPos);

					/** スタンさせる（StunStateが時間経過で自動解除する） */
					bear->GetEnemyStateMachine()->SetStun(true);

					/** 短いヒットストップ＋画面揺れで「弾いた！」を強調する */
					g_gameTime->StartSlowMotion(NULLIFY_HITSTOP_SCALE, NULLIFY_HITSTOP_DURATION);
					if (auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
						camera::GameCamera::ID()))
					{
						gameCamera->StartShake(NULLIFY_SHAKE_STRENGTH, NULLIFY_SHAKE_DURATION);
					}

					if (auto* lm = GameLogManager::GetInstance())
					{
						lm->QueueEvent({
							{ "ev",         "bear_attack_nullified" },
							{ "bear_id",    bear->GetLogId() },
							{ "penguin_id", enemy->m_foundPenguin->GetLogId() }
						});
					}

					enemy->m_foundPenguin = nullptr;
					return;
				}

				// 開始時にかまくらの中に「いなかった」場合のみ、即座にやられモーションに入れて足を止める
				if (!enemy->m_isTargetInsideIglooAtStart)
				{
					const int bearId = enemy->m_target->GetLogId();
					const int penguinId = enemy->m_foundPenguin->GetLogId();
					auto* status = enemy->m_foundPenguin->GetStateMachine()->GetChildPenguinStatus();
					if (status != nullptr && !status->IsDead())
					{
						auto* childSm= dynamic_cast<actor::ChildPenguinStateMachine*>(enemy->m_foundPenguin->GetStateMachine());
						if (childSm != nullptr)
						{
							childSm->SetAttackerEnemy(enemy->m_target);
						}

						// 生きている場合のみダメージ処理を実行
						enemy->m_foundPenguin->GetStateMachine()->Damage();

						if (auto* lm = GameLogManager::GetInstance())
						{
							lm->QueueEvent({ {"ev", "bear_attack"}, {"bear_id", bearId}, {"penguin_id", penguinId} });

							// 今の攻撃でトドメを刺したか判定
							if (status->IsDead())
							{
								lm->QueueEvent({ {"ev", "bear_kill"}, {"bear_id", bearId}, {"penguin_id", penguinId} });
								if (auto* am = app::achievement::AchievementManager::GetInstance())
									am->AddBearKill();
							}
						}
					}
				}

				// どちらにせよ一度攻撃態勢に入ったので、ターゲットはリセットして安全にする
				enemy->m_foundPenguin = nullptr;
			}

		}


		void EnemyController::ExitAttack(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();

			sm->SetActionButtonX(false);
			sm->SetIsNearPenguin(false);
		}


		int EnemyController::CheckAttack(EnemyController* enemy)
		{
			if (enemy->m_target->GetEnemyStateMachine()->IsPlayingAnimation())
			{
				return enEnemyState_Invalid;
			}

			// 満腹なら帰る
			if (enemy->m_isFull)
			{
				return enEnemyState_ReturnHome;
			}

			// まだターゲットいるなら追跡
			if (enemy->m_foundPenguin != nullptr)
			{
				return enEnemyState_Chase;
			}

			return enEnemyState_Idle;
		}


		/** 帰巣 */
		void EnemyController::EnterReturnHome(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetReturnHome(true);

			// スタック検出タイマーをリセット
			enemy->m_stuckTimer = 0.0f;
			enemy->m_lastCheckPosition = enemy->m_target->GetTransform().m_position;
		}


		void EnemyController::UpdateReturnHome(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;
			Vector3 toHome = enemy->m_target->GetHomePosition() - pos;

			// 到着判定
			if (toHome.LengthSq() < RETURN_HOME_STOP_DIST_SQ)
			{
				enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
				return;
			}

			toHome.Normalize();
			enemy->m_target->GetEnemyStateMachine()->SetMoveDirection(toHome);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(1.0f);

			// スタック検出：前回チェック位置からほとんど動いていない場合はタイマーを進める
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			Vector3 moved = pos - enemy->m_lastCheckPosition;
			moved.y = 0.0f;

			if (moved.LengthSq() < STUCK_MOVE_THRESHOLD * STUCK_MOVE_THRESHOLD)
			{
				enemy->m_stuckTimer += deltaTime;
			}
			else
			{
				// 動いていればタイマーをリセット
				enemy->m_stuckTimer = 0.0f;
				enemy->m_lastCheckPosition = pos;
			}
		}


		void EnemyController::ExitReturnHome(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetReturnHome(false);
			enemy->m_target->GetEnemyStateMachine()->SetDebuffReturnHome(false);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		int EnemyController::CheckReturnHome(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;
			Vector3 toHome = enemy->m_target->GetHomePosition() - pos;

			// 巣にある程度近づいたら巣の上にワープして睡眠へ遷移する
			if (toHome.LengthSq() < ARRIVE_DIST_RETURN_HOME * ARRIVE_DIST_RETURN_HOME)
			{
				// 巣の座標に強制移動してその場で寝かせる
				enemy->m_target->GetEnemyStateMachine()->SetPosition(enemy->m_target->GetHomePosition());
				return enEnemyState_CoolDown;
			}

			// スタック検出：一定時間動けていない場合は方向をリセットしてリトライ
			if (enemy->m_stuckTimer >= STUCK_TIME_LIMIT)
			{
				enemy->m_stuckTimer = 0.0f;
				enemy->m_lastCheckPosition = pos;

				// 移動入力をいったんリセットして再入力を促す
				enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
			}

			// まだ遠い → 継続
			return enEnemyState_ReturnHome;
		}


		/** クールダウン */
		void EnemyController::EnterCoolDown(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetCoolDown(true);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		void EnemyController::UpdateCoolDown(EnemyController* enemy)
		{}


		void EnemyController::ExitCoolDown(EnemyController* enemy)
		{
			enemy->m_coolDownTimer = 0.0f;

			enemy->m_eatCount = 0;
			enemy->m_isFull = false;

			enemy->m_target->GetEnemyStateMachine()->SetCoolDown(false);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		int EnemyController::CheckCoolDown(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();

			// EnemyCoolDownState::Update() 内で起床条件を満たした場合にフラグが折られる
			// 音で起きた場合は IsSearch() が true になっているのでSearchへ、そうでなければIdleへ
			if (!sm->IsCoolDown())
			{
				if (sm->IsSearch()) {
					if (auto* am = app::achievement::AchievementManager::GetInstance()) {
						// JSONのcondition名 "WakeUpBear" をハッシュ化して探す
						auto* baseAchieve = am->GetAchievement(Hash32("WakeUpBear"));
						if (auto* eventAchieve = dynamic_cast<app::achievement::EventAchievement*>(baseAchieve)) {
							eventAchieve->Unlock();
						}
					}

					return enEnemyState_Search;
				}
				return enEnemyState_Idle;
			}

			return enEnemyState_Invalid;
		}


		/** 咆哮 */
		void EnemyController::EnterRoar(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetIsRoar(true);

			// ペンギンを見つけた合図の「！」を出す。
			// 発見はどの経路（索敵・徘徊）でも必ずRoarを通るのでここが唯一の入口
			BattleManager::GetInstance().NotifyEnemyReactionChanged(
				enemy->m_target, ui::EnCPReactionType::Exclamation);

			// 咆哮の衝撃演出：プレイヤーの近くでの咆哮ほど強いラジアルブラーをかける
			Vector3 toDaddy = enemy->m_target->GetTransform().m_position
				- ChildPenguinManager::GetInstance()->GetDaddyPosition();
			toDaddy.y = 0.0f;
			const float distToDaddy = toDaddy.Length();
			if (distToDaddy <= ROAR_BLUR_MAX_DISTANCE)
			{
				// 至近で1.0、境界ぎりぎりで最小強度になるよう距離で線形に落とす
				const float closeness = 1.0f - (distToDaddy / ROAR_BLUR_MAX_DISTANCE);
				const float strength =
					ROAR_BLUR_MIN_STRENGTH + (1.0f - ROAR_BLUR_MIN_STRENGTH) * closeness;
				nsBeastEngine::g_renderingEngine->GetPostEffectManager()
					.GetRadialBlur().Start(strength, ROAR_BLUR_ATTACK_TIME, ROAR_BLUR_DURATION);

				// 近い咆哮ほど強い画面揺れも重ねる
				if (auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
					camera::GameCamera::ID()))
				{
					gameCamera->StartShake(ROAR_SHAKE_STRENGTH * closeness, ROAR_SHAKE_DURATION);
				}
			}
		}


		void EnemyController::UpdateRoar(EnemyController* enemy)
		{}


		void EnemyController::ExitRoar(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetIsRoar(false);
		}


		int EnemyController::CheckRoar(EnemyController* enemy)
		{
			if (!enemy->m_target->GetEnemyStateMachine()->IsPlayingAnimation())
			{
				return enEnemyState_Chase;
			}
			return enEnemyState_Invalid;
		}
	}
}