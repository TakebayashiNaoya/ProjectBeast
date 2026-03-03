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
		{
			m_fadeRender.Init("Assets/sprite/Loading.DDS", 1920.0f, 1080.0f);
		}


		Fade::~Fade()
		{

		}


		void Fade::Update()
		{
			if (m_state == FadeState::None) {
				return;
			}
			FadeProcess();
			m_fadeRender.Update();
		}


		void Fade::FadeProcess()
		{
			float delta = g_gameTime->GetFrameDeltaTime();

			if (m_state == FadeState::FadeIn)
			{
				m_timer -= delta;
				if (m_timer <= 0.0f)
				{
					m_timer = 0.0f;
					m_state = FadeState::None;
				}
			}
			else if (m_state == FadeState::FadeOut)
			{
				m_timer += delta;
				if (m_timer >= m_duration)
				{
					m_timer = m_duration;
				}
			}
		}


		void Fade::FadeIn(float duration)
		{
			m_state = FadeState::FadeIn;
			m_duration = duration;
			m_timer = duration;
		}


		void Fade::FadeOut(float duration)
		{
			m_state = FadeState::FadeOut;
			m_duration = duration;
			m_timer = 0.0f;
		}


		void Fade::Render(RenderContext& rc)
		{
			if (m_state == FadeState::None) {
				return;
			}
			float alpha = m_timer / m_duration;
			alpha = std::clamp(alpha, 0.0f, 1.0f);
			m_fadeRender.SetMulColor({ 1.0f,1.0f,1.0f,alpha });
			m_fadeRender.Draw(rc);
		}
	}
}