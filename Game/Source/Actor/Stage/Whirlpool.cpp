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
			IStageObject::Update();
		}


		void Whirlpool::Render(RenderContext& rc)
		{
			IStageObject::Render(rc);
		}
	}
}