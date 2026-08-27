/**
 * @file IKMath.h
 * @brief IK計算用の汎用ベクトル演算ヘルパー
 */
#pragma once


namespace app
{
	namespace actor
	{
		namespace ik
		{
			namespace
			{
				/** 2方向がほぼ同一とみなす内積のしきい値 */
				constexpr float PARALLEL_DOT_THRESHOLD = 0.9999f;
				/** 2方向がほぼ真逆とみなす内積のしきい値 */
				constexpr float ANTI_PARALLEL_DOT_THRESHOLD = -0.9999f;
				/** 真逆時に任意軸を選ぶ際、a.xがこの値未満ならX軸を基準軸候補にする */
				constexpr float ARBITRARY_AXIS_X_THRESHOLD = 0.9f;
			}

			/**
			 * @brief 外積 (a × b)
			 */
			inline Vector3 CrossProduct(const Vector3& a, const Vector3& b)
			{
				return Vector3(
					a.y * b.z - a.z * b.y,
					a.z * b.x - a.x * b.z,
					a.x * b.y - a.y * b.x
				);
			}

			/**
			 * @brief acosのNaN対策版（CharacterController.cppのdotYクランプと同じ考え方）
			 */
			inline float ClampedAcos(float x)
			{
				if (x > 1.0f) x = 1.0f;
				if (x < -1.0f) x = -1.0f;
				return acosf(x);
			}

			/**
			 * @brief ベクトル v を axis（正規化済み前提）周りに angle[rad] 回転させる
			 * @details ロドリゲスの回転公式
			 *  v' = v*cosθ + (axis×v)*sinθ + axis*(axis・v)*(1-cosθ)
			 */
			inline Vector3 RotateAroundAxis(const Vector3& v, const Vector3& axis, float angle)
			{
				float c = cosf(angle);
				float s = sinf(angle);

				Vector3 crossed = CrossProduct(axis, v);
				float dotted = axis.Dot(v);

				Vector3 result;
				result.x = v.x * c + crossed.x * s + axis.x * dotted * (1.0f - c);
				result.y = v.y * c + crossed.y * s + axis.y * dotted * (1.0f - c);
				result.z = v.z * c + crossed.z * s + axis.z * dotted * (1.0f - c);
				return result;
			}

			/**
			 * @brief row0/row1/row2（ボーンの姿勢を表す直交3軸）を、
			 *        oldDir方向を向いていたものが newDir方向を向くように
			 *        最小回転でまとめて回転させる
			 * @details 「アニメーションが向いていた方向」から「IKで向かせたい方向」への
			 *          差分回転だけを適用するので、ロール（ひねり）はアニメーションの
			 *          ものがそのまま活きます。ボーンのローカル軸がどれか（X/Y/Z軸のどれが
			 *          骨の長さ方向か）を知らなくても成立するのがポイントです。
			 */
			inline void RotateBasisTo(Vector3& row0, Vector3& row1, Vector3& row2,
				const Vector3& oldDir, const Vector3& newDir)
			{
				Vector3 a = oldDir;
				Vector3 b = newDir;
				if (a.LengthSq() < FLT_EPSILON || b.LengthSq() < FLT_EPSILON) {
					return;
				}
				a.Normalize();
				b.Normalize();

				float dot = a.Dot(b);
				if (dot > PARALLEL_DOT_THRESHOLD) {
					// ほぼ同じ方向。回転不要。
					return;
				}

				Vector3 axis;
				if (dot < ANTI_PARALLEL_DOT_THRESHOLD) {
					// ほぼ真逆（レアケース）。任意の直交軸を使って180度回転。
					Vector3 arbitrary = (fabsf(a.x) < ARBITRARY_AXIS_X_THRESHOLD) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
					axis = CrossProduct(a, arbitrary);
					if (axis.LengthSq() < FLT_EPSILON) {
						axis = Vector3(0.0f, 0.0f, 1.0f);
					}
					axis.Normalize();
					row0 = RotateAroundAxis(row0, axis, Math::PI);
					row1 = RotateAroundAxis(row1, axis, Math::PI);
					row2 = RotateAroundAxis(row2, axis, Math::PI);
					return;
				}

				axis = CrossProduct(a, b);
				axis.Normalize();
				float angle = ClampedAcos(dot);

				row0 = RotateAroundAxis(row0, axis, angle);
				row1 = RotateAroundAxis(row1, axis, angle);
				row2 = RotateAroundAxis(row2, axis, angle);
			}

			/**
			 * @brief oldDirからnewDirへの最小回転（回転軸・回転角）を求める
			 * @param oldDir  回転前の方向ベクトル
			 * @param newDir  回転後の方向ベクトル
			 * @param[out] outAxis  回転軸（正規化済み）
			 * @param[out] outAngle 回転角度[rad]
			 * @return 回転が必要な場合はtrue（ほぼ同方向で回転不要の場合はfalse）
			 */
			inline bool CalcMinimalRotation(const Vector3& oldDir, const Vector3& newDir,
				Vector3& outAxis, float& outAngle)
			{
				Vector3 a = oldDir;
				Vector3 b = newDir;
				if (a.LengthSq() < FLT_EPSILON || b.LengthSq() < FLT_EPSILON) {
					return false;
				}
				a.Normalize();
				b.Normalize();

				float dot = a.Dot(b);
				if (dot > PARALLEL_DOT_THRESHOLD) {
					// ほぼ同じ方向。回転不要。
					return false;
				}

				if (dot < ANTI_PARALLEL_DOT_THRESHOLD) {
					// ほぼ真逆（レアケース）。任意の直交軸を使って180度回転。
					Vector3 arbitrary = (fabsf(a.x) < ARBITRARY_AXIS_X_THRESHOLD) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
					outAxis = CrossProduct(a, arbitrary);
					if (outAxis.LengthSq() < FLT_EPSILON) {
						outAxis = Vector3(0.0f, 0.0f, 1.0f);
					}
					outAxis.Normalize();
					outAngle = Math::PI;
					return true;
				}

				outAxis = CrossProduct(a, b);
				outAxis.Normalize();
				outAngle = ClampedAcos(dot);
				return true;
			}

			/**
			 * @brief 位置と姿勢（基底3軸）を、pivotを中心にaxis周りへangle[rad]回転させる
			 * @param[in,out] pos  回転させる位置（ワールド座標）
			 * @param[in,out] row0,row1,row2 回転させる姿勢の基底3軸
			 * @param oldPivot 回転前のピボット位置
			 * @param newPivot 回転後のピボット位置
			 * @param axis     回転軸（正規化済み）
			 * @param angle    回転角度[rad]
			 */
			inline void RotateRigidAroundPivot(Vector3& pos, Vector3& row0, Vector3& row1, Vector3& row2,
				const Vector3& oldPivot, const Vector3& newPivot, const Vector3& axis, float angle)
			{
				Vector3 offset = pos - oldPivot;
				offset = RotateAroundAxis(offset, axis, angle);
				pos = newPivot + offset;

				row0 = RotateAroundAxis(row0, axis, angle);
				row1 = RotateAroundAxis(row1, axis, angle);
				row2 = RotateAroundAxis(row2, axis, angle);
			}
		}
	}
}
