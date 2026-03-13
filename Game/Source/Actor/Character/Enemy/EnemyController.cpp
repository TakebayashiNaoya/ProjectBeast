/**
 * @file EnemyController.cpp
 * @brief エネミーのコントローラー
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "EnemyControllerManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"


namespace app
{
	namespace actor
	{
		// static変数の初期化
		std::map<EnemyController::EnEnemyStateID, EnemyController::AIState> EnemyController::m_stateMap;


		EnemyController::EnemyController()
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

		}


		void EnemyController::UpdateIdle(EnemyController* enemy)
		{}


		void EnemyController::ExitIdle(EnemyController* enemy)
		{}


		int EnemyController::CheckIdle(EnemyController* enemy)
		{
			return enEnemyState_Invalid;
		}



		/** 移動 */
		void EnemyController::EnterWandering(EnemyController* enemy)
		{
			bool isXReverse = rand() % 2 >= 1;
			bool isZReverse = rand() % 2 >= 1;

			enemy->m_targetPosition = Vector3(
				static_cast<float>(rand() % 300) * (isXReverse ? 1.0f : -1.0f),
				0.0f,
				static_cast<float>(rand() % 300) * (isZReverse ? 1.0f : -1.0f)
			);
		}


		void EnemyController::UpdateWandering(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;

			// 目標方向
			Vector3 dir = enemy->m_targetPosition - pos;

			float distance = dir.Length();

			// まだ遠いなら移動
			if (distance > 0.1f)
			{
				dir.Normalize();

				float speed = 5.0f;

				pos += dir * speed * g_gameTime->GetFrameDeltaTime();
			}
		}


		void EnemyController::ExitWandering(EnemyController* enemy)
		{}


		int EnemyController::CheckWandering(EnemyController* enemy)
		{
			Vector3 pos = enemy->m_target->GetTransform().m_position;

			float distance = (enemy->m_targetPosition - pos).Length();

			// 到達したらIdleへ
			if (distance < 0.5f)
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

		}


		void EnemyController::ExitChase(EnemyController* enemy)
		{

		}


		int EnemyController::CheckChase(EnemyController* enemy)
		{
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
			return enEnemyState_Invalid;
		}
	}
}