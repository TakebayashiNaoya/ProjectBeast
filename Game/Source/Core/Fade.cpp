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
		Fade::Fade()
		{

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