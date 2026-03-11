/**
 * @file ChildPenguinBase.cpp
 * @brief 子ペンギンの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguinBase.h"


namespace app
{
	namespace actor
	{

		void ChildPenguinBase::Start()
		{
			// ペンギンの初期化
			PenguinBase::Init(PenguinBase::EnPenguinType::Child);
			// 基底クラスのStartを呼び出す
			PenguinBase::Start();
		}


		void ChildPenguinBase::Update()
		{
			PenguinBase::Update();
		}


		void ChildPenguinBase::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}


		void ChildPenguinBase::Init(const EnChildPenguinType childPenguinType)
		{
			// 子ペンギンの種類を保存
			m_childPenguinType = childPenguinType;
		}


		ChildPenguinBase::ChildPenguinBase()
			: m_childPenguinType(EnChildPenguinType::Max)
		{
		}
	}
}