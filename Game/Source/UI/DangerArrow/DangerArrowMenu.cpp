/**
 * @file DangerArrowMenu.cpp
 * @brief 危険矢印UI（1本分）のMenuクラス実装
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

            // パルスアニメーション: sin で 1±0.25 の範囲でスケールが振動する
            float pulseScale = 1.0f;
            if (m_isPulsing)
            {
                m_pulseTimer += g_gameTime->GetFrameDeltaTime() * 5.0f;
                pulseScale = 1.0f + 0.25f * sinf(m_pulseTimer);
            }
            else
            {
                m_pulseTimer = 0.0f;
            }

            m_arrowIcon->m_transform.m_localTransform.m_position =
                Vector3(m_arrowScreenPos.x, m_arrowScreenPos.y, 0.0f);
            m_arrowIcon->m_transform.m_localTransform.m_rotation = rot;
            m_arrowIcon->m_transform.m_localTransform.m_scale =
                Vector3(pulseScale, pulseScale, 1.0f);
        }
    }
}
