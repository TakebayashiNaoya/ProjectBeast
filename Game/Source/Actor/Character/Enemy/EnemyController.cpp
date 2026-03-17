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
			m_targetPosition(Vector3::Zero),
			isFind(false)
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

			/** 現在のステートのアップデート */
			currentState->update(this);

		}


		void EnemyController::Render(RenderContext& renderContext)
		{}


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


		/** サーチ */
		void EnemyController::EnterSearch(EnemyController* enemy)
		{

		}


		void EnemyController::UpdateSearch(EnemyController* enemy)
		{

		}


		void EnemyController::ExitSearch(EnemyController* enemy)
		{

		}


		int EnemyController::CheckSearch(EnemyController* enemy)
		{
			return enEnemyState_Wandering;
		}


		/** 徘徊 */
		void EnemyController::EnterWandering(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr)return;

			enemy->m_startPosition = enemy->m_target->GetTransform().m_position;

			bool x = rand() % 2 >= 1;
			bool z = rand() % 2 >= 1;

			Vector3 base = enemy->m_startPosition;

			enemy->m_targetPosition = base + Vector3(rand() % 50 * (x ? 1.0f : -1.0f), 0.0f, rand() % 50 * (z ? 1.0f : -1.0f));
		}


		void EnemyController::UpdateWandering(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			//Vector3 dis = enemy->m_targetPosition - enemy->m_target->GetTransform().m_position;
			//auto* stateMachine = enemy->m_target->GetEnemyStateMachine();

			//if (dis.Length() < 0.5f)
			//{
			//	// 新しい目的地
			//	bool x = rand() % 2 >= 1;
			//	bool z = rand() % 2 >= 1;

			//	Vector3 base = enemy->m_target->GetTransform().m_position;

			//	enemy->m_targetPosition = base + Vector3(
			//		rand() % 50 * (x ? 1.0f : -1.0f),
			//		0.0f,
			//		rand() % 50 * (z ? 1.0f : -1.0f)
			//	);

			//	return;
			//}

			//Vector3 dir = dis;
			//dir.Normalize();

			//stateMachine->SetMoveDirection(dir);
			//stateMachine->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitWandering(EnemyController* enemy)
		{
			if (enemy->m_target == nullptr) return;

			enemy->m_target->GetEnemyStateMachine()->SetStickLAmount(0.0f);

		}


		int EnemyController::CheckWandering(EnemyController* enemy)
		{
			//if (enemy->m_target == nullptr) return enEnemyState_Invalid;

			//Vector3 currentPos = enemy->m_target->GetTransform().m_position;

			//// 移動距離チェック
			//float moveDistance = (currentPos - enemy->m_startPosition).Length();

			//const float maxDistance = 10.0f;

			//if (moveDistance > maxDistance)
			//{
			//	return enEnemyState_Idle;
			//}

			//Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
			//Vector3 playerPos = EnemyControllerManager::GetInstance()->GetPlayer()->GetTransform().m_position;

			//float distance = (playerPos - enemyPos).Length();

			//// 発見距離
			//const float findDistance = 0.1f;

			//if (distance < findDistance)
			//{
			//	enemy->m_target->GetEnemyStateMachine()->SetIsFindPenguin(true);
			//	return enEnemyState_Chase;
			//}

			//return enEnemyState_Invalid;
			enemy->m_target->GetEnemyStateMachine()->SetIsFindPenguin(true);
			return enEnemyState_Chase;

		}


		/** チェイス */
		void EnemyController::EnterChase(EnemyController* enemy)
		{

		}


		void EnemyController::UpdateChase(EnemyController* enemy)
		{
			auto* player = EnemyControllerManager::GetInstance()->GetPlayer();

			if (player == nullptr) return;
			if (enemy->m_target == nullptr) return;

			Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
			Vector3 playerPos = player->GetTransform().m_position;

			Vector3 dis = playerPos - enemyPos;
			Vector3 dir = dis;
			dir.y = 0.0f;
			dir.Normalize();

			auto* stateMachine = enemy->m_target->GetEnemyStateMachine();

			stateMachine->SetMoveDirection(dir);
			stateMachine->SetStickLAmount(1.0f);
		}


		void EnemyController::ExitChase(EnemyController* enemy)
		{
			enemy->m_target->GetEnemyStateMachine()->SetMoveVector(Vector3::Zero);
		}


		int EnemyController::CheckChase(EnemyController* enemy)
		{
			auto* player = EnemyControllerManager::GetInstance()->GetPlayer();
			if (player == nullptr) return enEnemyState_Invalid;

			Vector3 enemyPos = enemy->m_target->GetTransform().m_position;
			Vector3 playerPos = player->GetTransform().m_position;

			float distance = (playerPos - enemyPos).Length();

			if (distance < 9.0f)
			{
				enemy->m_target->GetEnemyStateMachine()->SetActionButtonX(true);
				enemy->m_target->GetEnemyStateMachine()->SetIsNearPenguin(true);

				return enEnemyState_Attack;
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
		{}


		void EnemyController::ExitSwim(EnemyController* enemy)
		{}


		int EnemyController::CheckSwim(EnemyController* enemy)
		{
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