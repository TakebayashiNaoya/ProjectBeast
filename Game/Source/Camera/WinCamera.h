/**
 * @file CameraSteering.h
 * @brief カメラ操縦処理群
 * @author 藤谷
 */
#pragma once
#include "CameraCommon.h"


namespace app
{
	namespace actor
	{

	}

	namespace camera
	{
		/**
 * 勝利演出用カメラ（加速回転 → 正面停止）
 */
		class WinCamera : public ICameraController
		{
			appCameraController(WinCamera);

		private:
			CameraData m_data;
			Vector3 m_targetPos;     // プレイヤーの位置
			Vector3 m_targetFront;   // プレイヤーの正面方向

			float m_timer = 0.0f;
			float m_angle = 0.0f;        // 現在の回転角度
			float m_angularSpeed = 0.0f; // 現在の回転速度

			const float DURATION = 2.0f;  // 演出全体の時間（秒）
			const float ACCEL = 15.0f;    // 回転の加速度

		public:
			/**
			 * @param playerPos プレイヤーの座標
			 * @param playerFront プレイヤーが向いている方向ベクトル
			 */
			void SetTarget(const Vector3& playerPos, const Vector3& playerFront);

			void Update() override;

			const CameraData& GetCameraData() const override { return m_data; }
		};
	}
}