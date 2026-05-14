/**
 * @file Enemy.cpp
 * @brief エネミークラス
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			AnimationData ANIMATION_DATA[] =
			{
				{ "Assets/animData/bear/idle.tka", true },
				{ "Assets/animData/bear/idle_UnderWater.tka", true },
				{ "Assets/animData/bear/walk.tka", true },
				{ "Assets/animData/bear/attack.tka", false },
				{ "Assets/animData/bear/attack_UnderWater.tka", false },
				{ "Assets/animData/bear/backWalk.tka", true },
				{ "Assets/animData/bear/run.tka", true },
				{ "Assets/animData/bear/swim.tka", true },
				{ "Assets/animData/bear/buff.tka", false },
				{ "Assets/animData/bear/damage.tka", true },
				{ "Assets/animData/bear/eat.tka", true },
				{ "Assets/animData/bear/stun.tka", true },
				{ "Assets/animData/bear/sleep.tka", true },

			};


			ModelData ENEMY_MODEL_DATA =
			{
				"Assets/modelData/whiteBear/WhiteBear.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};

		}


		Enemy::Enemy()
		{
			Init(ENEMY_MODEL_DATA);

			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();

			m_stateMachine->Setup(this);
			m_characterStateMachine = m_stateMachine.get();
		}


		void Enemy::Start()
		{
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			m_stateMachine->Update();

			CharacterBase::Update();

			// 【デバッグ用・確認後に削除】
			// NaN発生時にどのステートフラグが立っているか出力する
			if (m_modelReady)
			{
				auto* sm = m_stateMachine.get();
				const float bx = m_modelRender.GetWorldAABBMin().x;
				if (std::isnan(bx))
				{
					K2_LOG(
						"Enemy NaN: isStun=%d isAttack=%d isRoar=%d isChasing=%d isSearch=%d isReturn=%d isCoolDown=%d isSwim=%d",
						sm->IsStun() ? 1 : 0,
						sm->IsAttack() ? 1 : 0,
						sm->IsRoar() ? 1 : 0,
						sm->IsChasing() ? 1 : 0,
						sm->IsSeach() ? 1 : 0,
						sm->IsReturnHome() ? 1 : 0,
						sm->IsCoolDown() ? 1 : 0,
						sm->IsSwim() ? 1 : 0
					);
				}
			}
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
