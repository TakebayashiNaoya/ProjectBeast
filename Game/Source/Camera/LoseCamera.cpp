/**
 * @file CameraSteering.cpp
 * @brief カメラのステアリング処理
 * @author 藤谷
 */
#include "stdafx.h"
#include "LoseCamera.h"


namespace
{
}


namespace app
{
	namespace camera
	{
		void LoseCamera::Update()
		{
			m_timer += 0.016f; // 本来はdeltaTimeを使用

			// 演出：ゆっくりと上昇しながら、キャラを見下ろす
			// 徐々に高く、遠くへ
			float height = 50.0f + (m_timer * 40.0f);
			float distance = 100.0f + (m_timer * 20.0f);

			m_data.position = m_targetPos + Vector3(0, height, -distance);
			m_data.target = m_targetPos; // 倒れたキャラを注視し続ける

			// 演出：少しずつ視野角(FOV)を広げて、孤独感を出す
			m_data.fov = Math::DegToRad(60.0f + (m_timer * 5.0f));
		}
	}
}