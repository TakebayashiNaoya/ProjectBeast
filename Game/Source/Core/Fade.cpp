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
			:m_IsEnable(false)
		{
			m_fadeRender.Init("Assets/sprite/Loading.DDS", 1920.0f, 1080.0f);
		}


		Fade::~Fade()
		{

		}


		void Fade::Update()
		{
			if (!m_IsEnable) {
				return;
			}
			m_fadeRender.Update();
		}


		void Fade::Render(RenderContext& rc)
		{
			if (!m_IsEnable) {
				return;
			}
			m_fadeRender.Draw(rc);
		}
	}
}