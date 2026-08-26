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

	//============================================//
	// 躍動感のためのカメラパラメーター
	// （2026-08-24 試遊フィードバック「カメラが単調」対応）
	//============================================//

	/** 位置のバネ追従の時定数（秒）。小さいほど機敏、大きいほどふわっと遅れて付いてくる */
	constexpr float POSITION_FOLLOW_TIME = 0.18f;
	/** 理想位置とこれ以上離れたらスナップする距離（ワープ・シーン切替対策） */
	constexpr float POSITION_SNAP_DISTANCE = 600.0f;

	/** 速度の平滑化の時定数（秒）。FOV・先読み・低アングルの入力になる */
	constexpr float SPEED_SMOOTH_TIME = 0.25f;
	/** 速度演出が始まる速度と最大になる速度（ユニット/秒）。歩き90〜スライド最大300が目安 */
	constexpr float SPEED_EFFECT_MIN = 110.0f;
	constexpr float SPEED_EFFECT_MAX = 270.0f;

	/** 視野角（度）。基準値から、最高速時は最大値まで広がる（疾走感）。
	 *  FOVの変化は酔いのトリガーになるため、振れ幅は小さく・変化はゆっくりにする
	 *  （2026-08-25 酔い対策で 70°/0.3s → 65°/0.5s に緩和） */
	constexpr float FOV_BASE_DEG = 60.0f;
	constexpr float FOV_MAX_DEG = 65.0f;
	/** 視野角の追従の時定数（秒） */
	constexpr float FOV_SMOOTH_TIME = 0.5f;

	/** 先読み：注視点を移動方向へずらす最大距離と、追従の時定数（秒） */
	constexpr float LOOKAHEAD_MAX_DISTANCE = 55.0f;
	constexpr float LOOKAHEAD_SMOOTH_TIME = 0.4f;

	/** 最高速時にカメラ高さを下げる量（低アングルで地面の流れを強調する）。
	 *  上下動は酔いやすいため控えめにする（2026-08-25 酔い対策で 30 → 15） */
	constexpr float HIGH_SPEED_HEIGHT_DROP = 15.0f;

	/**
	 * @brief 時定数ベースの平滑化係数を求める
	 * @details 1 - exp(-dt/τ)。フレームレートに依存しない追従率になる
	 */
	float SmoothFactor(const float deltaTime, const float timeConstant)
	{
		if (timeConstant <= 0.0f) return 1.0f;
		return 1.0f - expf(-deltaTime / timeConstant);
	}
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

			//============================================//
			// ここから躍動感の後処理（速度推定 → 先読み → FOV → バネ追従）
			//============================================//

			/** ターゲットの速度を位置差分から推定して平滑化する。
			 *  ステートマシンに依存しないので、どのキャラを追っていても機能する */
			const Vector3 rawTargetPos = m_targetCharacter->GetTransform().m_position;
			if (!m_isPositionInitialized)
			{
				m_prevTargetPos = rawTargetPos;
			}
			Vector3 moved = rawTargetPos - m_prevTargetPos;
			moved.y = 0.0f;
			const float rawSpeed = (deltaTime > FLT_EPSILON) ? moved.Length() / deltaTime : 0.0f;
			m_prevTargetPos = rawTargetPos;
			m_smoothedSpeed += (rawSpeed - m_smoothedSpeed) * SmoothFactor(deltaTime, SPEED_SMOOTH_TIME);

			/** 速度演出の強さ（0〜1）。歩きでは0、スライド最高速で1になる */
			const float speedT = std::clamp(
				(m_smoothedSpeed - SPEED_EFFECT_MIN) / (SPEED_EFFECT_MAX - SPEED_EFFECT_MIN),
				0.0f, 1.0f);

			/** 先読み：移動方向へ注視点をずらす（平滑化つき） */
			Vector3 lookAheadTarget = Vector3::Zero;
			if (moved.LengthSq() > FLT_EPSILON && speedT > 0.0f)
			{
				Vector3 moveDir = moved;
				moveDir.Normalize();
				lookAheadTarget = moveDir * (LOOKAHEAD_MAX_DISTANCE * speedT);
			}
			m_lookAheadOffset +=
				(lookAheadTarget - m_lookAheadOffset) * SmoothFactor(deltaTime, LOOKAHEAD_SMOOTH_TIME);

			/** 視野角：速度に応じて広げる（疾走感） */
			const float fovTarget = Math::DegToRad(
				FOV_BASE_DEG + (FOV_MAX_DEG - FOV_BASE_DEG) * speedT);
			if (m_currentFov <= 0.0f) m_currentFov = fovTarget;
			m_currentFov += (fovTarget - m_currentFov) * SmoothFactor(deltaTime, FOV_SMOOTH_TIME);

			/** 理想のカメラ位置：高速時は少し低いアングルから */
			const Vector3 lookTarget = targetPosition + m_lookAheadOffset;
			Vector3 idealPosition = lookTarget + m_toVector;
			idealPosition.y -= HIGH_SPEED_HEIGHT_DROP * speedT;

			/** 位置のバネ追従。加速すると親が画面の先へ引っ張り、止まるとふわっと収まる。
			 *  初回とワープ級の移動はスナップして、カメラの長距離飛行を防ぐ */
			if (!m_isPositionInitialized
				|| (idealPosition - m_smoothedPosition).LengthSq()
					> POSITION_SNAP_DISTANCE * POSITION_SNAP_DISTANCE)
			{
				m_smoothedPosition = idealPosition;
				m_isPositionInitialized = true;
			}
			else
			{
				m_smoothedPosition +=
					(idealPosition - m_smoothedPosition) * SmoothFactor(deltaTime, POSITION_FOLLOW_TIME);
			}

			// 最終的なカメラ位置と注視点をセット
			nextData.position = m_smoothedPosition;
			nextData.target = lookTarget;
			nextData.fov = m_currentFov;
			data = nextData;
		}
	}
}