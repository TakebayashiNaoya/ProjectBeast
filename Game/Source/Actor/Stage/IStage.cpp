/**
 * @file IStage.cpp
 * @brief ステージの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"


namespace app
{
	namespace actor
	{
		void IStageObject::Start()
		{
			/** 物理判定を作成 */
			m_physicsStaticObject.CreateFromModel(m_modelRender.GetModel(), m_modelRender.GetModel().GetWorldMatrix());
		}


		void IStageObject::Update()
		{
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();
		}


		void IStageObject::Render(RenderContext& rc)
		{
			m_modelRender.Draw(rc);
		}

		void IStageObject::Init(const char* fileName)
		{
			m_modelRender.Init(fileName);
		}
	}
}