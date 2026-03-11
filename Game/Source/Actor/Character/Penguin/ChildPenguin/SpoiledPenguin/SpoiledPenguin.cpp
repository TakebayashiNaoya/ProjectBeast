/**
 * @file SpoiledPenguin.cpp
 * @brief 甘えん坊ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "SpoiledPenguin.h"
 //#include "SpoiledPenguinStateMachine.h"
 //#include "SpoiledPenguinStatus.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const Vector3 SCALE = { 0.3f, 0.3f, 0.3f };
		}


		SpoiledPenguin::SpoiledPenguin()
		{
			//m_stateMachine = std::make_unique<SpoiledPenguinStateMachine>(this);
			//m_status = std::make_unique<SpoiledPenguinStatus>();
			m_status->Setup();
			SetScale(SCALE);
		}


		void SpoiledPenguin::Start()
		{
			ChildPenguinBase::Init(ChildPenguinBase::EnChildPenguinType::Spoiled);
			ChildPenguinBase::Start();
		}


		void SpoiledPenguin::Update()
		{
			//m_stateMachine->Update();

			ChildPenguinBase::Update();
		}


		void SpoiledPenguin::Render(RenderContext& rc)
		{
			ChildPenguinBase::Render(rc);
		}
	}
}