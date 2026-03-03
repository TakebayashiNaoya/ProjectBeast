/**
 * @file Fade.cpp
 * @brief ロード画面を表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "Fade.h"


namespace app
{
	namespace core
	{
		Fade* Fade::m_instance = nullptr;


		Fade::Fade()
			:m_isEnable(false)
		{
			m_fadeRender.Init("Assets/sprite/Loading.DDS", 1920.0f, 1080.0f);
		}


		Fade::~Fade()
		{

		}


		void Fade::Update()
		{
			if (!m_isEnable) {
				return;
			}
			FadeProcess();
			m_fadeRender.Update();
		}


		void Fade::FadeProcess()
		{
			float delta = g_gameTime->GetFrameDeltaTime();

			if (m_isFadeIn)
			{
				m_timer -= delta;
				if (m_timer <= 0.0f)
				{
					m_timer = 0.0f;
					m_isFadeIn = false;
					m_isEnable = false;
				}
			}
			else if (m_isFadeOut)
			{
				m_timer += delta;
				if (m_timer >= m_duration)
				{
					m_timer = m_duration;
					m_isFadeOut = false;
				}
			}
		}


		void Fade::FadeIn(float duration)
		{
			m_duration = duration;
			m_timer = 0.0f;
			m_isFadeOut = false;
			m_isFadeIn = true;
			m_isEnable = true;
		}


		void Fade::FadeOut(float duration)
		{
			m_duration = duration;
			m_timer = 0.0f;
			m_isFadeOut = true;
			m_isFadeIn = false;
			m_isEnable = true;
		}


		void Fade::Render(RenderContext& rc)
		{
			if (!m_isEnable) {
				return;
			}
			m_fadeRender.Draw(rc);
		}
	}
}