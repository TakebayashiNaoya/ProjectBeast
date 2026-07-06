/**
 * @file FrontChecker.cpp
 * @brief 前方チェッククラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "FrontChecker.h"


namespace app
{
	namespace ui
	{
		bool FrontChecker::IsInFront(
			const Vector3& basePosition,
			const Quaternion& baseRotation,
			const Vector3& targetPosition,
			float dotThreshold)
		{
			// カメラの前方向ベクトルを取得
			const Vector3 cameraFront = CameraSystem::Get().GetMainCamera().GetForward();


			// y座標を0にしてXZ平面上のベクトルにする
			const Vector3 base2D = Vector3(basePosition.x, 0.0f, basePosition.z);
			const Vector3 target2D = Vector3(targetPosition.x, 0.0f, targetPosition.z);

			Vector3 toTargetNorm = target2D - base2D;

			// 同一座標などでゼロベクトルになる場合はNormalizeしない（前方扱いにしておく）
			if (toTargetNorm.LengthSq() <= FLT_EPSILON)
			{
				return true;
			}
			toTargetNorm.Normalize();

			// 基準の前方向ベクトル（こちらもXZ平面に投影しておくと安全）
			Vector3 front = Vector3::Front;
			baseRotation.Apply(front);
			front.y = 0.0f;
			front.Normalize();

			const float dot = front.Dot(toTargetNorm);
			if (dot < dotThreshold)
			{
				return false;
			}

			// カメラの前方向についてもXZ平面上で判定する
			Vector3 cameraFront2D = Vector3(cameraFront.x, 0.0f, cameraFront.z);

			// カメラが真上・真下を向いている等でゼロベクトルになる場合はカメラ判定をスキップする
			if (cameraFront2D.LengthSq() > FLT_EPSILON)
			{
				cameraFront2D.Normalize();

				const float cameraDot = cameraFront2D.Dot(toTargetNorm);
				if (cameraDot < dotThreshold)
				{
					return false;
				}
			}

			return true;
		}
	}
}