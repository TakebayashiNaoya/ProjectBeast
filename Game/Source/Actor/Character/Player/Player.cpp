/**
 * @file Player.cpp
 * @brief プレイヤークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "Player.h"
#include "PlayerStateMachine.h"
#include "PlayerStatus.h"
#include "Source/Actor/ActorStatus.h"

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


			ModelData MODEL_DATA =
			{
				"Assets/modelData/unityChan.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisY,
				std::size(ANIMATION_DATA)
			};

		}


		Player::Player()
		{
			m_stateMachine = std::make_unique<PlayerStateMachine>(this);
			m_status = std::make_unique<PlayerStatus>();
			m_status->Setup();
		}


		void Player::Start()
		{
			Init(MODEL_DATA);
			CharacterBase::Start();
		}


		void Player::Update()
		{
			//m_stateMachine->Update();

			CharacterBase::Update();
		}


		void Player::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}