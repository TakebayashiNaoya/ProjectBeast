/**
 * @file ClumsyPenguin.cpp
 * @brief おっちょこちょいペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ClumsyPenguin.h"
 //#include "ClumsyPenguinStateMachine.h"
 //#include "ClumsyPenguinStatus.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const Vector3 SCALE = { 0.3f, 0.3f, 0.3f };
		}


		ClumsyPenguin::ClumsyPenguin()
		{
			//m_stateMachine = std::make_unique<ClumsyPenguinStateMachine>(this);
			//m_status = std::make_unique<ClumsyPenguinStatus>();
			m_status->Setup();
			SetScale(SCALE);
		}


		void ClumsyPenguin::Start()
		{
			ChildPenguinBase::Init(ChildPenguinBase::EnChildPenguinType::Clumsy);
			ChildPenguinBase::Start();
		}


		void ClumsyPenguin::Update()
		{
			//m_stateMachine->Update();

			ChildPenguinBase::Update();
		}


		void ClumsyPenguin::Render(RenderContext& rc)
		{
			ChildPenguinBase::Render(rc);
		}
	}
}