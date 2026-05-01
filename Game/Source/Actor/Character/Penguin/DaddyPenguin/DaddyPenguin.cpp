/**
 * @file DaddyPenguin.cpp
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinController.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Graphics/PBRStatus.h"


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
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};
		}


		DaddyPenguin::DaddyPenguin()
		{
			CharacterBase::Init(MODEL_DATA);

			m_status = std::make_unique<DaddyPenguinStatus>();
			m_status->Setup();
			m_stateMachine = std::make_unique<DaddyPenguinStateMachine>(this);
			m_characterStateMachine = m_stateMachine.get();

			// コントローラーの作成
			m_controller = std::make_unique<DaddyPenguinController>(this);
		}


		void DaddyPenguin::Start()
		{

			PenguinBase::Start();
		}


		void DaddyPenguin::Update()
		{
			if (m_controller)
			{
				m_controller->Update();
			}

			m_stateMachine->Update();

			PenguinBase::Update();

			/** 泳ぎ中はモデルの描画位置のみY座標をオフセットする */
			/** 物理・ステート判定には影響を与えない */
			if (m_stateMachine->IsSwimming() && m_modelReady)
			{
				Vector3 renderPos = m_transform.m_position;
				renderPos.y += SWIM_Y_OFFSET;
				m_modelRender.SetTRS(renderPos, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
			}
		}


		void DaddyPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}