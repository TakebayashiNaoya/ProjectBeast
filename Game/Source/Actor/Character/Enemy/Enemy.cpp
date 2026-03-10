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
		Enemy::Enemy()
		{
			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();
		}


		void Enemy::Start()
		{
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
