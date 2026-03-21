/**
 * @file CameraSteering.cpp
 * @brief カメラのステアリング処理
 * @author 藤谷
 */
#include "stdafx.h"
#include "CameraSteering.h"
#include "Source/Actor/Character/CharacterBase.h"


namespace
{
}


namespace app
{
	namespace camera
	{
		void CameraSteering::Update(CameraData& data, const float deltaTime)
		{
			if (m_targetCharacter == nullptr) {
				return;
			}
			CameraData nextData = data;


			// 理想のカメラ位置を計算（ターゲットの後ろ・上）
			// ※簡易的にZ軸手前に引いていますが、本来はターゲットの向き(Rotation)も考慮して回転させます
			Vector3 targetPosition = m_targetCharacter->GetTransform().m_position;
			targetPosition.y += 50.0f;

			Vector3 position = m_targetCharacter->GetTransform().m_position + m_toVector;

			nextData.position = position;
			nextData.target = targetPosition;

			// 右スティックで回転
			Vector3 rotationVector = Vector3(g_pad[0]->GetRStickXF(), g_pad[0]->GetRStickYF(), 0.0f);
			if (rotationVector.LengthSq() > FLT_EPSILON) {
				rotationVector.x *= m_config.rotationSpeedX;
				rotationVector.y *= m_config.rotationSpeedY;
				// rotXでY軸回転
				Quaternion yRotation;
				yRotation.SetRotationY(-rotationVector.x);
				yRotation.Apply(m_toVector);
				// rotYでXZ軸回転
				Quaternion xzRotation;
				xzRotation.SetRotation(g_camera3D->GetRight(), -rotationVector.y);
				xzRotation.Apply(m_toVector);
			}

			nextData.position = m_targetCharacter->GetTransform().m_position + m_toVector;
			data = nextData;
		}
	}
}