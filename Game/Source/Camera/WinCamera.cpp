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
		void WinCamera::SetTarget(const Vector3& playerPos, const Vector3& playerFront)
		{
			m_targetPos = playerPos;
			m_targetFront = playerFront;
			m_timer = 0.0f;
			m_angle = 0.0f;
			m_angularSpeed = 0.0f; // 停止状態から開始
		}


		void WinCamera::Update()
		{
			float deltaTime = 0.016f; // 本来はマネージャー等から取得
			m_timer += deltaTime;

			if (m_timer < DURATION) {
				// 1. 加速的に回転速度を上げる
				m_angularSpeed += ACCEL * deltaTime;
				m_angle += m_angularSpeed * deltaTime;
			}
			else {
				// 2. 終了時間は正面で固定
				// プレイヤーの正面(m_targetFront)の逆方向にカメラを置くと「正面から映す」ことになる
				m_angle = atan2f(-m_targetFront.x, -m_targetFront.z);
			}

			// 3. カメラ座標の計算（円運動）
			float radius = 150.0f; // プレイヤーとの距離
			float height = 50.0f;  // カメラの高さ

			m_data.position.x = m_targetPos.x + sinf(m_angle) * radius;
			m_data.position.z = m_targetPos.z + cosf(m_angle) * radius;
			m_data.position.y = m_targetPos.y + height;

			// 4. 注視点はプレイヤーの顔あたり
			m_data.target = m_targetPos + Vector3(0, 40.0f, 0);
		}
	}
}