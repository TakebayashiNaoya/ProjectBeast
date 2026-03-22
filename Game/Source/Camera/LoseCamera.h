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
		class LoseCamera : public ICameraController {
			appCameraController(LoseCamera);
		private:
			CameraData m_data;
			float m_timer = 0.0f;
			Vector3 m_targetPos; // 勝利したキャラの座標

		public:
			void SetTarget(const Vector3& pos) { m_targetPos = pos; }

			void Update() override;

			const CameraData& GetCameraData() const override { return m_data; }
		};
	}
}