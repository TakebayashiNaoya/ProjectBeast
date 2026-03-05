/**
 * @file Enemy.cpp
 * @brief エネミークラス
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyStateMachine.h"


namespace app
{
	namespace actor
	{
		Enemy::Enemy()
		{
			m_stateMachine = std::make_unique<EnemyStateMachine>(this);

			m_status->Setup();
		}


		void Enemy::Start()
		{
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			CharacterBase::Update();
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
