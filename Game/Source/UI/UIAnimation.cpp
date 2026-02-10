/**
 * @file UIAnimation.cpp
 * @brief UIAnimationのクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		UIColorAnimation::UIColorAnimation()
		{
			SetFunc([&](Vector4 v)
				{
					m_ui->m_color = v;
				});
		}





		/******************************************/


		UIScaleAnimation::UIScaleAnimation()
		{
			SetFunc([&](Vector3 s)
				{
					m_ui->m_transform.m_localTransform.m_scale = s;
				});
		}





		/******************************************/


		UITranslateAnimation::UITranslateAnimation()
		{
			SetFunc([&](Vector3 s)
				{
					m_ui->m_transform.m_localTransform.m_position = s;
				});
		}





		/******************************************/


		UITranslateOffsetAnimation::UITranslateOffsetAnimation()
		{
			SetFunc([&](Vector3 offset)
				{
					m_ui->m_transform.m_localTransform.m_position.Add(offset);
				});
		}





		/******************************************/


		UIRotationAnimation::UIRotationAnimation()
		{
			SetFunc([&](float s)
				{
					m_ui->m_transform.m_localTransform.m_rotation.SetRotationDegZ(s);
				});
		}
	}
}