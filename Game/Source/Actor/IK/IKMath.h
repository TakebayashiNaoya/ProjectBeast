/**
 * @file IKMath.h
 * @brief IK計算用の汎用ベクトル演算ヘルパー
 * @details
 *  Vector3/Matrix側に同名のメンバー関数（Cross等）が既にあるかもしれませんが、
 *  未確認のAPIには依存せず、x/y/z の直接アクセスと四則演算子（+ - * のみ確認済み）と
 *  .Dot() / .Normalize() / .Length() / .LengthSq()（すべてCharacterController.cppで
 *  使用実績あり）だけで組んであります。
 *  もし Vector3::Cross() 等が既にあれば、そちらに差し替えてもらって構いません。
 */
#pragma once

namespace app
{
	namespace actor
	{
		namespace ik
		{
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
				if (dot > 0.9999f) {
					// ほぼ同じ方向。回転不要。
					return;
				}

				Vector3 axis;
				if (dot < -0.9999f) {
					// ほぼ真逆（レアケース）。任意の直交軸を使って180度回転。
					Vector3 arbitrary = (fabsf(a.x) < 0.9f) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
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
				if (dot > 0.9999f) {
					// ほぼ同じ方向。回転不要。
					return false;
				}

				if (dot < -0.9999f) {
					// ほぼ真逆（レアケース）。任意の直交軸を使って180度回転。
					Vector3 arbitrary = (fabsf(a.x) < 0.9f) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
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
