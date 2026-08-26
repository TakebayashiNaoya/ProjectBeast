#include "stdafx.h"
#include "CameraController.h"
#include "CameraManager.h"
#include <algorithm> // std::clamp用


namespace app
{
	namespace camera
	{
		namespace
		{
			//============================================//
			// ショーケースカメラのパラメーター
			//============================================//

			/** 一周にかける時間（秒）＝動画1ループの長さ。
			 *  動画はループ再生されるため、ちょうど一周で撮って切れば継ぎ目が見えない */
			constexpr float SHOWCASE_ORBIT_PERIOD = 33.0f;
			/** 周回半径。流氷原の外縁（R_FIELD 3000）の少し内側から見下ろす */
			constexpr float SHOWCASE_ORBIT_RADIUS = 2300.0f;
			/** カメラ高度の基準値。ペンギンが見える低さまで下げつつ島の起伏はかわす */
			constexpr float SHOWCASE_HEIGHT_BASE = 420.0f;
			/** カメラ高度のうねりの振幅（単調な周回にしないための上下動） */
			constexpr float SHOWCASE_HEIGHT_WAVE = 80.0f;
			/** 高度のうねりの周期（秒）。ループの継ぎ目を消すため一周あたり整数回
			 *  （SHOWCASE_ORBIT_PERIOD / 2 で2回）うねって開始時と同じ状態に戻す */
			constexpr float SHOWCASE_HEIGHT_WAVE_PERIOD = SHOWCASE_ORBIT_PERIOD / 2.0f;
			/** 注視点の高さ。地表の少し上を見て氷壁と空も画面に入れる */
			constexpr float SHOWCASE_TARGET_HEIGHT = 100.0f;

			//============================================//
			// 画面揺れのパラメーター
			// （2026-08-25 酔い対策。GDC2016 Eiserloh "Juicing Your Cameras" 系の定石に準拠）
			//============================================//

			/** 揺れの合成sin波の角周波数（rad/秒）。約13Hzと8Hz。
			 *  非整数比の2波を混ぜると周期性が見えず自然なガタつきになる */
			constexpr float SHAKE_FREQ_MAIN = 82.0f;
			constexpr float SHAKE_FREQ_SUB = 51.0f;
			/** 縦揺れの倍率。上下動は横揺れより酔いやすいため弱める */
			constexpr float SHAKE_VERTICAL_RATE = 0.7f;
		}


		void GameCamera::Update()
		{
			/** SetState() で入った今フレームの姿勢に、揺れとパンチインを上乗せする。
			 *  m_data は毎フレーム SetState() で置き換わるため、オフセットは蓄積しない */
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// パンチイン：sinカーブで注視点へ寄って戻る
			if (m_punchTimer > 0.0f)
			{
				m_punchTimer -= deltaTime;
				const float progress = 1.0f - (m_punchTimer / m_punchDuration);
				const float envelope = sinf(Math::PI * std::clamp(progress, 0.0f, 1.0f));
				m_data.position += (m_data.target - m_data.position) * (m_punchAmount * envelope);
			}

			// 画面揺れ：滑らかなノイズを画面の右・上方向にだけ乗せる。
			// 毎フレームの乱数は60Hzの不連続なガタつきになり酔いやすいため、
			// 非整数比の2つのsin波を合成した連続的な揺れにする（Perlinノイズの簡易版）。
			// 前後（視線）方向の揺れはズームの脈動に見えるため入れない。
			// 減衰は残量の2乗（trauma方式）で、終わり際ほど素早く収まる
			if (m_shakeTimer > 0.0f)
			{
				m_shakeTimer -= deltaTime;
				m_shakeTime += deltaTime;

				const float decay = (m_shakeDuration > 0.0f)
					? std::clamp(m_shakeTimer / m_shakeDuration, 0.0f, 1.0f) : 0.0f;
				const float amplitude = m_shakeStrength * decay * decay;

				// 画面基準の右・上ベクトルを注視方向から作る
				Vector3 forward = m_data.target - m_data.position;
				if (forward.LengthSq() > FLT_EPSILON)
				{
					forward.Normalize();
					Vector3 right;
					right.Cross(Vector3::Up, forward);
					right.Normalize();
					Vector3 up;
					up.Cross(forward, right);

					const float t = m_shakeTime;
					const float noiseX = 0.6f * sinf(t * SHAKE_FREQ_MAIN)
						+ 0.4f * sinf(t * SHAKE_FREQ_SUB + 1.7f);
					const float noiseY = 0.6f * sinf(t * SHAKE_FREQ_MAIN * 1.31f + 4.2f)
						+ 0.4f * sinf(t * SHAKE_FREQ_SUB * 0.79f + 2.3f);

					m_data.position += right * (noiseX * amplitude)
						+ up * (noiseY * amplitude * SHAKE_VERTICAL_RATE);
				}
			}
			else
			{
				m_shakeTime = 0.0f;
			}
		}


		void ShowcaseCamera::Update()
		{
			m_time += g_gameTime->GetFrameDeltaTime();

			/** デバッグ用の注視点指定（BEAST_SHOWCASE_LOOKAT="x,z,半径,高さ"）。
			 *  イグルーの接地やキャラの見た目を接写で確認するためのスイッチ。
			 *  未指定ならステージ全景の周回（動画撮影用） */
			static bool s_lookAtParsed = false;
			static bool s_hasLookAt = false;
			static float s_lookX = 0.0f;
			static float s_lookZ = 0.0f;
			static float s_lookRadius = 500.0f;
			static float s_lookHeight = 250.0f;
			if (!s_lookAtParsed)
			{
				s_lookAtParsed = true;
				char buf[64];
				size_t len = 0;
				if (getenv_s(&len, buf, sizeof(buf), "BEAST_SHOWCASE_LOOKAT") == 0 && len > 0)
				{
					s_hasLookAt = sscanf_s(buf, "%f,%f,%f,%f",
						&s_lookX, &s_lookZ, &s_lookRadius, &s_lookHeight) >= 2;
				}
			}

			if (s_hasLookAt)
			{
				const float angle = 2.0f * Math::PI * m_time / 12.0f;	// 接写は12秒で一周
				const float groundY = SHOWCASE_TARGET_HEIGHT;
				m_data.position = Vector3(
					s_lookX + s_lookRadius * cosf(angle),
					groundY + s_lookHeight,
					s_lookZ + s_lookRadius * sinf(angle)
				);
				m_data.target = Vector3(s_lookX, groundY, s_lookZ);
				return;
			}

			const float angle = 2.0f * Math::PI * m_time / SHOWCASE_ORBIT_PERIOD;
			const float height = SHOWCASE_HEIGHT_BASE
				+ SHOWCASE_HEIGHT_WAVE * sinf(2.0f * Math::PI * m_time / SHOWCASE_HEIGHT_WAVE_PERIOD);

			m_data.position = Vector3(
				SHOWCASE_ORBIT_RADIUS * cosf(angle),
				height,
				SHOWCASE_ORBIT_RADIUS * sinf(angle)
			);
			m_data.target = Vector3(0.0f, SHOWCASE_TARGET_HEIGHT, 0.0f);
		}


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