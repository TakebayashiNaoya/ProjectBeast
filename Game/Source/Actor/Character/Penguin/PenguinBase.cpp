/**
 * @file PenguinBase.cpp
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinBase.h"


namespace app
{
	namespace actor
	{
		void PenguinBase::Start()
		{
			CharacterBase::Start();
		}


		void PenguinBase::Update()
		{
			CharacterBase::Update();
		}


		void PenguinBase::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}