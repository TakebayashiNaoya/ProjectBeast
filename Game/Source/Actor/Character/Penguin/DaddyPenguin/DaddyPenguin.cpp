/**
 * @file DaddyPenguin.cpp
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"


namespace app
{
	namespace actor
	{

		DaddyPenguin::DaddyPenguin()
		{
			m_stateMachine = std::make_unique<DaddyPenguinStateMachine>(this);
			m_status = std::make_unique<DaddyPenguinStatus>();
			m_status->Setup();
		}


		void DaddyPenguin::Start()
		{
			PenguinBase::Init(EnPenguinType::Daddy);
			PenguinBase::Start();
		}


		void DaddyPenguin::Update()
		{
			m_stateMachine->Update();

			PenguinBase::Update();
		}


		void DaddyPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}