/**
 * @file DangerArrowMenu.cpp
 * @brief 危険矢印UI（1本分）のMenuクラス実装
 * @author 竹林
 */
#include "stdafx.h"
#include "DangerArrowMenu.h"


namespace app
{
    namespace ui
    {
        void DangerArrowMenu::InitializeLogic()
        {
            m_arrowIcon = GetUI<UIIcon>(Hash32("arrow"));
            m_isVisible = false;

            if (m_arrowIcon)
            {
                m_arrowIcon->m_isDraw = false;
            }
        }


        void DangerArrowMenu::Update()
        {
            UpdateArrowTransform();
            Base::Update();
        }


        void DangerArrowMenu::UpdateArrowTransform()
        {
            if (!m_arrowIcon) return;

            m_arrowIcon->m_isDraw = m_isVisible;

            if (!m_isVisible) return;

            Quaternion rot;
            rot.SetRotationZ(m_arrowAngleRad);

            m_arrowIcon->m_transform.m_localTransform.m_position =
                Vector3(m_arrowScreenPos.x, m_arrowScreenPos.y, 0.0f);
            m_arrowIcon->m_transform.m_localTransform.m_rotation = rot;
        }
    }
}
