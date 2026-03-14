/**
 * @file DaddyPenguin.cpp
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/daddyPenguin/DaddyPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisY,
				std::size(ANIMATION_DATA)
			};
		}


		DaddyPenguin::DaddyPenguin()
		{
			m_status = std::make_unique<DaddyPenguinStatus>();
			m_status->Setup();
			m_stateMachine = std::make_unique<DaddyPenguinStateMachine>(this);
		}


		void DaddyPenguin::Start()
		{
			CharacterBase::Init(MODEL_DATA);
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