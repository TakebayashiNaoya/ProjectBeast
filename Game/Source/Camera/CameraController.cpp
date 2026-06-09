#include "stdafx.h"
#include "CameraController.h"
#include "CameraManager.h"
#include <algorithm> // std::clamp用


namespace app
{
	namespace camera
	{

#if defined(APP_DEBUG)
		void DebugCamera::OnEnter()
		{
			m_cameraData = CameraManager::Get().GetCurrentCameraData();
		}


		void DebugCamera::Update()
		{
			// fov調整
			if (g_pad[0]->IsPress(enButtonRB1)) {
				float value = g_pad[0]->GetLStickYF();
				m_cameraData.fov += value * 0.05f;
				return;
			}

			// 左スティックで移動
			{
				Vector3 inputDirection;
				inputDirection.x = g_pad[0]->GetLStickXF();
				inputDirection.z = g_pad[0]->GetLStickYF();

				// カメラの前方向と右方向のベクトルを取得
				Vector3 forward = CameraSystem::Get().GetMainCamera().GetForward();
				Vector3 right = CameraSystem::Get().GetMainCamera().GetRight();

				// y方向には移動しない
				forward.y = 0.0f;
				right.y = 0.0f;

				// 左スティックの入力量を加算
				right *= inputDirection.x;
				forward *= inputDirection.z;

				Vector3 direction = right + forward;
				direction.Normalize();
				// 移動速度調整
				direction.Scale(10.0f);

				// 平行移動
				m_cameraData.position += direction;
				m_cameraData.target += direction;
			}
			// 右スティックで回転
			{
				float rotX = g_pad[0]->GetRStickXF() * 0.05f;
				float rotY = g_pad[0]->GetRStickYF() * 0.05f;

				// 【修正1】インバート修正（マイナスを外しました）
				// rotXでY軸回転
				Quaternion yRotation;
				yRotation.SetRotationY(rotX);
				Vector3 toVector = m_cameraData.position - m_cameraData.target;
				yRotation.Apply(toVector);

				// rotYでXZ軸回転
				Vector3 rightDir;
				rightDir.Cross(Vector3::Up, toVector); // Vector3クラスのメンバ関数を使用
				rightDir.Normalize();

				Quaternion xzRotation;
				xzRotation.SetRotation(rightDir, rotY);
				xzRotation.Apply(toVector);

				// 【修正2】クランプ処理（無限回転防止）
				float length = toVector.Length();
				float maxAngle = Math::DegToRad(85.0f);  // デバッグ用なので制限は緩め
				float minAngle = Math::DegToRad(-85.0f);

				float maxY = sinf(maxAngle) * length;
				float minY = sinf(minAngle) * length;

				toVector.y = std::clamp(toVector.y, minY, maxY);

				float xzLenSq = length * length - toVector.y * toVector.y;
				float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;

				Vector3 xzDir;
				xzDir.Set(toVector.x, 0.0f, toVector.z);

				if (xzDir.LengthSq() > FLT_EPSILON) {
					xzDir.Normalize();
				}
				else {
					xzDir.Set(0.0f, 0.0f, -1.0f);
				}

				toVector.x = xzDir.x * xzLen;
				toVector.z = xzDir.z * xzLen;

				m_cameraData.position = m_cameraData.target + toVector;
			}
		}
#endif
	}
}