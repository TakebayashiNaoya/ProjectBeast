/**
 * @file Whirlpool.cpp
 * @brief 渦潮クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "Whirlpool.h"


namespace app
{
	namespace actor
	{
		void Whirlpool::Start()
		{
			Init("Assets/modelData/stage/Whirlpool/whirlpool.tkm");
		}


		void Whirlpool::Update()
		{
			// 渦潮を回転させる
			m_transform.m_rotation.AddRotationDegY(3.0f);

			IStageObject::Update();
		}


		void Whirlpool::Render(RenderContext& rc)
		{
			IStageObject::Render(rc);
		}
	}
}