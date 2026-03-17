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
				{ "Assets/animData/idle.tka", true },
				{ "Assets/animData/walk.tka", true },
				{ "Assets/animData/run.tka", true },
				{ "Assets/animData/jump.tka", false }
			};


			ModelData ENEMY_MODEL_DATA =
			{
				"Assets/modelData/penguin/daddyPenguin/DaddyPenguin.tkm",
				//"Assets/modelData/unityChan.tkm",
				//ANIMATION_DATA,
				nullptr,
				EnModelUpAxis::enModelUpAxisY,
				//std::size(ANIMATION_DATA)
				0
			};

		}


		Enemy::Enemy()
		{
			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();
		}


		void Enemy::Start()
		{
			Init(ENEMY_MODEL_DATA);
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			m_stateMachine->Update();

			CharacterBase::Update();
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
