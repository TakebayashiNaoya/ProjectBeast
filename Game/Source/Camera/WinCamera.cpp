/**
 * @file CameraSteering.cpp
 * @brief カメラのステアリング処理
 * @author 藤谷
 */
#include "stdafx.h"
#include "WinCamera.h"


namespace
{
}


namespace app
{
	namespace camera
	{
		void WinCamera::Update()
		{
			m_timer += 0.016f; // 本来はdeltaTimeを使用

			// 例：キャラの周りを円運動しながら、少しずつ上昇する演出
			float radius = 200.0f;
			float speed = 10.0f;

			m_data.position.x = m_targetPos.x + cos(m_timer * speed) * radius;
			m_data.position.z = m_targetPos.z + sin(m_timer * speed) * radius;
			m_data.position.y = m_targetPos.y + 50.0f + (m_timer * 10.0f); // 徐々に上がる

			m_data.target = m_targetPos + Vector3(0, 40.0f, 0); // 顔あたりを注視
		}
	}
}