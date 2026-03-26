/**
 * @file EnemyController.cpp
 * @brief エネミーのコントローラー
 * @author 立山
 */
#include "stdafx.h"
#include <time.h>

#include "Enemy.h"
#include "EnemyController.h"
#include "EnemyControllerManager.h"

#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Player/Player.h"
#include "Source/Actor/Stage/StageSystem.h"


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
			, m_homePosition(Vector3::Zero)
			, m_lastKnownPenguinPos(Vector3::Zero)
			, isFind(false)
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
		{
			static bool ini = false;
			if (!ini)
			{
				Initialize();
				ini = true;
			}

			EnemyControllerManager::GetInstance()->Register(this);
		}


		EnemyController::~EnemyController()
		{
			EnemyControllerManager::GetInstance()->UnRegister(this);
		}


		bool EnemyController::Start()
		{
			return true;
		}


		void EnemyController::Update()
		{
			if (!m_isHomeInitialized)
			{
				Vector3 pos = StageSystem::GetInstance()->GetObjectPosition("bearHome");

				if (pos.LengthSq() > 0.0001f)
				{
					m_homePosition = pos;
					m_isHomeInitialized = true;
				}
			}

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
			ChildPenguin* found = FindTarget();
			if (found != nullptr && m_currentState != enEnemyState_CoolDown)
			{
				m_foundPenguin = found;
				m_lastKnownPenguinPos = found->GetTransform().m_position;
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
				Vector3 diff = penguin->GetTransform().m_position - m_target->GetTransform().m_position;
				diff.y = 0.0f;
				if (diff.LengthSq() > 600.0f * 600.0f)// 距離外ならIdleへ
				{
					continue;
				}

				diff.Normalize();
				auto moveDirection = m_target->GetEnemyStateMachine()->GetMoveDirection();
				float cosv = moveDirection.Dot(diff);
				float cosAngle = cosf(Math::PI / 180.0f * 70.0f);
				if (cosv >= cosAngle)
				{
					return penguin;
				}
			}
			return nullptr;
		}


		bool EnemyController::IsFarFromHome()const
		{
			Vector3 pos = m_target->GetTransform().m_position;
			Vector3 toHome = m_homePosition - pos;

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
			//enemy->m_target->GetEnemyStateMachine()->SetMoveDirection(Vector3::Zero);
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

			const float idleTime = static_cast<float>(rand() % 500) * 0.01f;
			if (enemy->m_elapsedTime > idleTime) {
				if (rand() % 10 >= 5) {
					return enEnemyState_Wandering;
				}
				return enEnemyState_Search;
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



		/** サーチ */
		void EnemyController::EnterSearch(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			enemy->m_target->GetEnemyStateMachine()->SetSeach(true);

			enemy->m_elapsedTime = 0.0f;

			// 左右どちらに回るかランダム
			enemy->m_searchDir = (rand() % 2 == 0) ? 1 : -1;

			// 回転速度（ラジアン）
			enemy->m_searchSpeed = Math::PI / 2.0f; // 90度/秒くらい

			Quaternion q = enemy->m_target->GetTransform().m_rotation;

			Vector3 forward;
			forward.x = 2.0f * (q.x * q.z + q.w * q.y);
			forward.y = 0.0f;
			forward.z = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);

			if (forward.LengthSq() < 0.001f)
			{
				forward = Vector3::AxisZ;
			}

			forward.Normalize();

			enemy->m_searchAngle = atan2f(forward.x, forward.z);


			// 移動は止める
			auto* sm = enemy->m_target->GetEnemyStateMachine();
			sm->SetStickLAmount(0.0f);
			enemy->m_prePosition = enemy->m_target->GetTransform().m_position;
		}


		void EnemyController::UpdateSearch(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			float delta = g_gameTime->GetFrameDeltaTime();
			enemy->m_elapsedTime += delta;

			// 角度更新
			enemy->m_searchAngle += enemy->m_searchSpeed * enemy->m_searchDir * delta;

			// 向きをベクトルに変換
			Vector3 dir;
			dir.x = sinf(enemy->m_searchAngle);
			dir.z = cosf(enemy->m_searchAngle);
			dir.y = 0.0f;

			dir.Normalize();

			auto* sm = enemy->m_target->GetEnemyStateMachine();
			sm->SetMoveDirection(dir);

			// 移動はしない
			sm->SetStickLAmount(0.01f);

			enemy->m_target->GetEnemyStateMachine()->SetPosition(enemy->m_prePosition);
		}


		void EnemyController::ExitSearch(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
			enemy->m_target->GetEnemyStateMachine()->SetSeach(false);
		}


		int EnemyController::CheckSearch(EnemyController* enemy)
		{
			// ターゲット見つけたらチェイス
			if (enemy->m_foundPenguin != nullptr)
			{
				return enEnemyState_Chase;
			}
			if (enemy->IsFarFromHome())
			{
				return enEnemyState_ReturnHome;
			}

			// 一定時間で終了
			if (enemy->m_elapsedTime > 2.0f)
			{
				return enEnemyState_Idle;
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
			//if (enemy->IsFarFromHome())
			//{
			//	return enEnemyState_ReturnHome;
			//}

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

			//enemy->m_attackTimer = 0.0f;
			//enemy->m_isAttacking = true;

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
			Vector3 toHome = enemy->m_homePosition - pos;

			// 到着判定
			if (toHome.LengthSq() < 10.0f)
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
			Vector3 toHome = enemy->m_homePosition - pos;

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

			enemy->m_coolDownTimer += g_gameTime->GetFrameDeltaTime();

			if (enemy->m_coolDownTimer >= 5.0f)
			{
				return enEnemyState_Idle;
			}
			return enEnemyState_Invalid;
		}

	}
}