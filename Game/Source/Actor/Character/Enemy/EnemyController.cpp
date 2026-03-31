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

#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Noise/NoiseManager.h"


namespace app
{
	namespace actor
	{
		// static変数の初期化
		std::map<EnemyController::EnEnemyStateID, EnemyController::AIState> EnemyController::m_stateMap;


		EnemyController::EnemyController()
			:m_target(nullptr)
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
			, m_searchTimer(0.0f) // 初期化追加
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
				ChangeState((EnEnemyStateID)nextState);
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
				m_currentState == enEnemyState_Chase)
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
				if (diff.LengthSq() > 600.0f * 600.0f)// 距離外ならIdleへ
				{
					continue;
				}

				diff.Normalize();
				auto moveDirection = m_target->GetEnemyStateMachine()->GetMoveDirection();
				float cosv = moveDirection.Dot(diff);
				float cosAngle = cosf(Math::PI / 180.0f * 70.0f);
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


		bool EnemyController::IsFarFromHome()const
		{
			Vector3 pos = m_target->GetTransform().m_position;
			Vector3 toHome = m_target->GetHomePosition() - pos;

			const float MAX_DIST = 500.0f;
			if (toHome.LengthSq() > MAX_DIST * MAX_DIST)
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
			if (enemy->m_target->GetEnemyStateMachine()->IsSeach())
			{
				return enEnemyState_Search;
			}

			const float idleTime = static_cast<float>(rand() % 500) * 0.01f;
			if (enemy->m_elapsedTime > idleTime) {
				// 時間経過によるランダム遷移は、(0,0,0)へ行かないようWanderingに統一
				return enEnemyState_Wandering;
			}
			return enEnemyState_Invalid;
		}


		/** スタン */
		void EnemyController::EnterStun(EnemyController* enemy)
		{

		}


		void EnemyController::UpdateStun(EnemyController* enemy)
		{

		}


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

			enemy->m_target->GetEnemyStateMachine()->SetSeach(true);
			enemy->m_searchTimer = 0.0f; // タイマーリセット
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
			const float SEARCH_THRESHOLD = 15.0f; // 閾値

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
			if (distanceSq > 5.0f * 5.0f) // 目標までまだ遠い場合（距離5.0f以内で到着とする）
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
				if (enemy->m_searchTimer > 3.0f)
				{
					sm->SetSeach(false); // 諦めフラグを落とす
				}
			}
		}


		void EnemyController::ExitSearch(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
			enemy->m_target->GetEnemyStateMachine()->SetSeach(false);
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
			if (!enemy->m_target->GetEnemyStateMachine()->IsSeach()) {
				return enEnemyState_Wandering;
			}

			return enEnemyState_Invalid;
		}


		/** 徘徊 */
		void EnemyController::EnterWandering(EnemyController* enemy)
		{

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
			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			// 耳で音を検知してフラグが立ったら、索敵ステートへ移行
			if (enemy->m_target->GetEnemyStateMachine()->IsSeach()) {
				return enEnemyState_Search;
			}

			Vector3 distance = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex] - enemy->m_target->GetTransform().m_position;

			if (distance.Length() <= 20.0f)
			{
				return enEnemyState_Idle;
			}
			return enEnemyState_Invalid;
		}


		/** チェイス */
		void EnemyController::EnterChase(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(true);
		}


		void EnemyController::UpdateChase(EnemyController* enemy)
		{
			Vector3 enemyPos = enemy->m_target->GetTransform().m_position;

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

			if (dir.LengthSq() > 0.001f)
			{
				dir.Normalize();
			}

			auto* sm = enemy->m_target->GetEnemyStateMachine();
			sm->SetMoveDirection(dir);
			sm->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitChase(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(false);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		int EnemyController::CheckChase(EnemyController* enemy)
		{
			if (enemy->m_foundPenguin != nullptr)
			{
				Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
				Vector3 penguinPos = enemy->m_foundPenguin->GetTransform().m_position;

				Vector3 diff = penguinPos - enemyPos;
				diff.y = 0.0f;

				const float LOST_DIST = 800.0f;

				if (diff.LengthSq() > LOST_DIST * LOST_DIST)
				{
					enemy->m_foundPenguin = nullptr;
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
				const float ATTACK_DIST = 80.0f;
				if (distSq <= ATTACK_DIST * ATTACK_DIST)
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
				const float ARRIVE_DIST = 20.0f;
				if (diff.LengthSq() <= ARRIVE_DIST * ARRIVE_DIST)
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
			// 対象座標までの距離
			Vector3 distance = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex] - enemy->m_target->GetTransform().m_position;
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
			if (enemy->m_foundPenguin != nullptr)
			{
				return enEnemyState_Chase;
			}
			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			Vector3 distance = enemy->m_wanderingPosList[enemy->m_wanderingPosListIndex] - enemy->m_target->GetTransform().m_position;

			if (distance.Length() <= 20.0f)
			{
				return enEnemyState_Idle;
			}
			return enEnemyState_Invalid;
		}



		/** 攻撃 */
		void EnemyController::EnterAttack(EnemyController* enemy)
		{
			auto* sm = enemy->m_target->GetEnemyStateMachine();

			sm->SetActionButtonX(true);
			sm->SetIsNearPenguin(true);

			// 満腹処理
			enemy->m_eatCount++;
			if (enemy->m_eatCount >= enemy->m_maxEatCount)
			{
				enemy->m_isFull = true;
			}
		}


		void EnemyController::UpdateAttack(EnemyController* enemy)
		{

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
		}


		void EnemyController::UpdateReturnHome(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;
			Vector3 toHome = enemy->m_target->GetHomePosition() - pos;

			// 到着判定
			if (toHome.LengthSq() < 200.0f)
			{
				enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
				return;
			}

			toHome.Normalize();
			enemy->m_target->GetEnemyStateMachine()->SetMoveDirection(toHome);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitReturnHome(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetReturnHome(false);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		int EnemyController::CheckReturnHome(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;
			Vector3 toHome = enemy->m_target->GetHomePosition() - pos;

			// 到着したら終了
			if (toHome.LengthSq() < 50.0f * 50.0f)
			{
				return enEnemyState_CoolDown;
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
			// 音で起きた場合は IsSeach() が true になっているのでSearchへ、そうでなければIdleへ
			if (!sm->IsCoolDown())
			{
				if (sm->IsSeach()) {
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
		}


		void EnemyController::UpdateRoar(EnemyController* enemy)
		{

		}


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