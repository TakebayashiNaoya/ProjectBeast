/**
 * @file SeriousPenguin.cpp
 * @brief 真面目ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "SeriousPenguin.h"
 //#include "SeriousPenguinStateMachine.h"
 //#include "SeriousPenguinStatus.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const Vector3 SCALE = { 0.3f, 0.3f, 0.3f };
		}


		SeriousPenguin::SeriousPenguin()
		{
			//m_stateMachine = std::make_unique<SeriousPenguinStateMachine>(this);
			//m_status = std::make_unique<SeriousPenguinStatus>();
			m_status->Setup();
			SetScale(SCALE);
		}


		void SeriousPenguin::Start()
		{
			ChildPenguinBase::Init(ChildPenguinBase::EnChildPenguinType::Serious);
			ChildPenguinBase::Start();
		}


		void SeriousPenguin::Update()
		{
			//m_stateMachine->Update();

			ChildPenguinBase::Update();
		}


		void SeriousPenguin::Render(RenderContext& rc)
		{
			ChildPenguinBase::Render(rc);
		}
	}
}