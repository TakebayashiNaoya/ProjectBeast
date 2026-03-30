/**
 * @file CameraSteering.cpp
 * @brief カメラのステアリング処理
 * @author 藤谷
 */
#include "stdafx.h"
#include "CameraSteering.h"
#include "Source/Actor/Character/CharacterBase.h"
#include <algorithm> // std::clamp用


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


			// 理想のカメラ位置（注視点）を計算：ターゲットの少し上
			Vector3 targetPosition = m_targetCharacter->GetTransform().m_position;
			targetPosition.y += 50.0f;

			// 右スティックで回転
			Vector3 rotationVector = Vector3(g_pad[0]->GetRStickXF(), g_pad[0]->GetRStickYF(), 0.0f);
			if (rotationVector.LengthSq() > FLT_EPSILON) {
				rotationVector.x *= m_config.rotationSpeedX;
				rotationVector.y *= m_config.rotationSpeedY;

				// 【修正1】インバート修正（元のマイナス符号を外して回転方向を逆にしました）
				float rotX = rotationVector.x;
				float rotY = rotationVector.y;

				// rotXでY軸回転（左右の回転）
				Quaternion yRotation;
				yRotation.SetRotationY(rotX);
				yRotation.Apply(m_toVector);

				// rotYでXZ軸回転（上下の回転）
				// カメラから見た正確な「右方向」のベクトルを算出して回転軸にする
				Vector3 rightDir;
				rightDir.Cross(Vector3::Up, m_toVector); // Vector3クラスのメンバ関数を使用
				rightDir.Normalize();

				Quaternion xzRotation;
				xzRotation.SetRotation(rightDir, rotY);

				Vector3 nextToVector = m_toVector;
				xzRotation.Apply(nextToVector);

				// 【修正2】上下の角度制限（クランプ処理）
				float length = nextToVector.Length();

				// 制限したい角度（度数法） ※必要に応じてこの数値を調整してください
				float maxAngle = Math::DegToRad(80.0f);   // 見下ろしの最大角度
				float minAngle = Math::DegToRad(-20.0f);  // 見上げの最大角度（地面にめり込まない程度）

				float maxY = sinf(maxAngle) * length;
				float minY = sinf(minAngle) * length;

				// Y座標を制限範囲内に収める
				nextToVector.y = std::clamp(nextToVector.y, minY, maxY);

				// Y座標を制限した分、ターゲットとの距離(length)が変わらないようにXZ平面の長さを再計算して補正する
				float xzLenSq = length * length - nextToVector.y * nextToVector.y;
				float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;

				Vector3 xzDir;
				xzDir.Set(nextToVector.x, 0.0f, nextToVector.z);

				if (xzDir.LengthSq() > FLT_EPSILON) {
					xzDir.Normalize();
				}
				else {
					xzDir.Set(0.0f, 0.0f, -1.0f); // 真上や真下を向いた時の安全対策
				}

				nextToVector.x = xzDir.x * xzLen;
				nextToVector.z = xzDir.z * xzLen;

				m_toVector = nextToVector;
			}

			// 最終的なカメラ位置と注視点をセット
			nextData.position = targetPosition + m_toVector;
			nextData.target = targetPosition;
			data = nextData;
		}
	}
}