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
				{ "Assets/animData/bear/walk.tka", true },
				{ "Assets/animData/bear/attack.tka", true },

			};


			ModelData ENEMY_MODEL_DATA =
			{
				"Assets/modelData/whiteBear/WhiteBear.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisY,
				std::size(ANIMATION_DATA)
			};

		}


		Enemy::Enemy()
		{
			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();

			m_stateMachine->Setup(this);
		}


		void Enemy::Start()
		{
			Init(ENEMY_MODEL_DATA);
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			m_stateMachine->Update();

			Vector3 move = m_stateMachine->GetMoveVector();
			if (move.Length() > 0.001f)
			{
				Vector3 currentPos = GetTransform().m_position;
				Vector3 targetPos = currentPos + move;

				const Vector3& position = m_characterController.Execute(targetPos, 1.0f);

				Quaternion rot = GetTransform().m_rotation;
				rot.SetRotationYFromDirectionXZ(move);
				SetPosition(position);
				SetRotation(rot);
			}
			CharacterBase::Update();
			m_stateMachine->SetPosition(GetTransform().m_position);
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
