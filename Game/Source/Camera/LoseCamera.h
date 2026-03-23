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
		 * @brief 敗北演出用カメラ
		 */
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




		/****************************************************/



		/**
		 * 敗北演出用カメラ（上空俯瞰・静止）
		 */
		class DefeatCamera : public ICameraController
		{
			appCameraController(DefeatCamera);

		private:
			CameraData m_data;

		public:
			/**
			 * @brief 倒れたプレイヤーの位置を元に、カメラの停止位置を計算する
			 * @param playerPos プレイヤーの座標
			 */
			void SetTarget(const Vector3& playerPos);

			void OnEnter() override;

			void Update() override;

			const CameraData& GetCameraData() const override { return m_data; }
		};
	}
}