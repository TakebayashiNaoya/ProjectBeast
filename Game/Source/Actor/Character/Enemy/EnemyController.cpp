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
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Player/Player.h"


namespace app
{
	namespace actor
	{
		// static変数の初期化
		std::map<EnemyController::EnEnemyStateID, EnemyController::AIState> EnemyController::m_stateMap;


		EnemyController::EnemyController()
			:m_target(nullptr),
			m_elapsedTime(0.0f),
			m_prePosition(Vector3::Zero),
			m_startPosition(Vector3::Zero),
			isFind(false),
			m_isStun(false)
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
			FindTarget();
		}


		void EnemyController::Render(RenderContext& renderContext)
		{}


		void EnemyController::AddTargetPos(const Vector3& pos)
		{
			m_targetPosList.push_back(pos);
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
			auto penguinList = actor::ChildPenguinManager::GetInstance()->GetChildPenguiin();
			for (auto* penguin : penguinList)
			{
				Vector3 diff = penguin->GetTransform().m_position - m_target->GetTransform().m_position;
				diff.y = 0.0f;
				if (diff.LengthSq() > 700.0f * 700.0f)// 距離外ならIdleへ
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
		}


		/** 各ステートの処理はこの下に書いていく */


		/** 待機 */
		void EnemyController::EnterIdle(EnemyController* enemy)
		{
			enemy->m_elapsedTime = 0.0f;

			auto* sm = enemy->m_target->GetEnemyStateMachine();

			// 完全停止
			sm->SetStickLAmount(0.0f);
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
			//// ターゲット見つけたらチェイス
			//if (enemy->FindTarget() != nullptr)
			//{
			//	//enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(true);
			//	return enEnemyState_Chase;
			//}

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
			Vector3 distance = enemy->m_targetPosList[enemy->m_targetPosListIndex] - enemy->m_target->GetTransform().m_position;
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

			enemy->m_targetPosListIndex++;
			if (enemy->m_targetPosListIndex >= enemy->m_targetPosList.size())
			{
				enemy->m_targetPosListIndex = 0;
			}
		}


		int EnemyController::CheckWandering(EnemyController* enemy)
		{
			Vector3 distance = enemy->m_targetPosList[enemy->m_targetPosListIndex] - enemy->m_target->GetTransform().m_position;

			if (distance.Length() <= 20.0f)
			{
				return enEnemyState_Idle;
			}
			return enEnemyState_Invalid;
		}


		/** チェイス */
		void EnemyController::EnterChase(EnemyController* enemy)
		{

		}


		void EnemyController::UpdateChase(EnemyController* enemy)
		{
			//auto* player = EnemyControllerManager::GetInstance()->GetPlayer();

			//if (player == nullptr) return;
			//if (enemy->m_target == nullptr) return;

			//Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
			//Vector3 playerPos = player->GetTransform().m_position;

			//Vector3 dis = playerPos - enemyPos;
			//Vector3 dir = dis;
			//dir.y = 0.0f;
			//dir.Normalize();

			//auto* stateMachine = enemy->m_target->GetEnemyStateMachine();

			//stateMachine->SetMoveDirection(dir);
			//stateMachine->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitChase(EnemyController* enemy)
		{
			//enemy->m_target->GetEnemyStateMachine()->SetActionButtonB(true);
			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);
		}


		int EnemyController::CheckChase(EnemyController* enemy)
		{
			//auto* player = EnemyControllerManager::GetInstance()->GetPlayer();
			//if (player == nullptr) return enEnemyState_Invalid;

			//Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
			//Vector3 playerPos = player->GetTransform().m_position;

			//float distance = (playerPos - enemyPos).Length();

			//if (distance < 9.0f)
			//{
			//	enemy->m_target->GetEnemyStateMachine()->SetActionButtonX(true);
			//	enemy->m_target->GetEnemyStateMachine()->SetIsNearPenguin(true);

			//	return enEnemyState_Attack;
			//}
			return enEnemyState_Idle;
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
			Vector3 distance = enemy->m_targetPosList[enemy->m_targetPosListIndex] - enemy->m_target->GetTransform().m_position;
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

			enemy->m_targetPosListIndex++;
			if (enemy->m_targetPosListIndex >= enemy->m_targetPosList.size())
			{
				enemy->m_targetPosListIndex = 0;
			}
		}


		int EnemyController::CheckSwim(EnemyController* enemy)
		{
			Vector3 distance = enemy->m_targetPosList[enemy->m_targetPosListIndex] - enemy->m_target->GetTransform().m_position;

			if (distance.Length() <= 20.0f)
			{
				return enEnemyState_Idle;
			}
			return enEnemyState_Invalid;
		}



		/** 攻撃 */
		void EnemyController::EnterAttack(EnemyController* enemy)
		{}


		void EnemyController::UpdateAttack(EnemyController* enemy)
		{}


		void EnemyController::ExitAttack(EnemyController* enemy)
		{}


		int EnemyController::CheckAttack(EnemyController* enemy)
		{
			return enEnemyState_Chase;
			return enEnemyState_Invalid;
		}
	}
}