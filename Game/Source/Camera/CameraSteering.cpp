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
	const Vector3 TARGET_OFFSET = Vector3(0.0f, 50.0f, 0.0f);	// ターゲットの注視点オフセット（例: ターゲットの頭上50ユニット）
	constexpr float MAX_VERTICAL_ANGLE = 80.0f;					// 上下の回転の最大角度（例: 80度）
	constexpr float MIN_VERTICAL_ANGLE = 0.0f;					// 上下の回転の最小角度（例: 0度、地面にめり込まない程度）
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
			targetPosition.Add(TARGET_OFFSET);

			// 右スティックで回転（二乗カーブで少し倒したときは緩やか、深く倒したときは速くなる）
			const float rawX = g_pad[0]->GetRStickXF();
			const float rawY = g_pad[0]->GetRStickYF();
			Vector3 rotationVector = Vector3(rawX * fabsf(rawX), rawY * fabsf(rawY), 0.0f);
			if (rotationVector.LengthSq() > FLT_EPSILON) {
				rotationVector.x *= m_config.rotationSpeedX * deltaTime;
				rotationVector.y *= m_config.rotationSpeedY * deltaTime;

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
				float maxAngle = Math::DegToRad(MAX_VERTICAL_ANGLE);   // 見下ろしの最大角度
				float minAngle = Math::DegToRad(MIN_VERTICAL_ANGLE);  // 見上げの最大角度（地面にめり込まない程度）

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