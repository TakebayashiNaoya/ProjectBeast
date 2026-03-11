/**
 * @file ChildPenguin.cpp
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/childPenguin/ChildPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisY,
				std::size(ANIMATION_DATA)
			};
		}


		ChildPenguin::ChildPenguin()
		{
			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this);
			m_status = std::make_unique<ChildPenguinStatus>();
			m_status->Setup();
		}


		void ChildPenguin::Start()
		{
			CharacterBase::Init(MODEL_DATA);
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			m_stateMachine->Update();

			PenguinBase::Update();
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}