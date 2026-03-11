/**
 * @file CaringPenguin.cpp
 * @brief 世話焼きペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CaringPenguin.h"
 //#include "CaringPenguinStateMachine.h"
 //#include "CaringPenguinStatus.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const Vector3 SCALE = { 0.3f, 0.3f, 0.3f };
		}


		CaringPenguin::CaringPenguin()
		{
			//m_stateMachine = std::make_unique<CaringPenguinStateMachine>(this);
			//m_status = std::make_unique<CaringPenguinStatus>();
			m_status->Setup();
			SetScale(SCALE);
		}


		void CaringPenguin::Start()
		{
			ChildPenguinBase::Init(ChildPenguinBase::EnChildPenguinType::Caring);
			ChildPenguinBase::Start();
		}


		void CaringPenguin::Update()
		{
			//m_stateMachine->Update();

			ChildPenguinBase::Update();
		}


		void CaringPenguin::Render(RenderContext& rc)
		{
			ChildPenguinBase::Render(rc);
		}
	}
}