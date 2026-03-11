/**
 * @file ChildPenguinBase.cpp
 * @brief 子ペンギンの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"


namespace app
{
	namespace actor
	{

		void ChildPenguin::Start()
		{
			// ペンギンの初期化
			PenguinBase::Init(PenguinBase::EnPenguinType::Child);
			// 基底クラスのStartを呼び出す
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			PenguinBase::Update();
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}


		ChildPenguin::ChildPenguin()
			: m_childPenguinType(EnChildPenguinType::Max)
		{}
	}
}