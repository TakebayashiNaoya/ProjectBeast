/**
 * @file TwoBoneLegIK.cpp
 * @brief 2ボーン解析式IK（脚1本分）の実装
 */
#include "stdafx.h" // ※プロジェクトのプリコンパイル済みヘッダ名に合わせて調整してください
#include "../../../../k2EngineLow/graphics/Skeleton.h"
#include "IKMath.h"
#include "TwoBoneLegIK.h"
#include <cfloat>
#include <cmath>

namespace app
{
	namespace actor
	{
		namespace ik
		{
			namespace
			{
				/** ワールド行列からVector3の平行移動成分を取り出す（v[3]はVector4想定） */
				inline Vector3 GetTranslation(const Matrix& m)
				{
					return Vector3(m.v[3].x, m.v[3].y, m.v[3].z);
				}

				/** ワールド行列から3本の基底ベクトルをVector3として取り出す */
				inline void GetBasisRows(const Matrix& m, Vector3& row0, Vector3& row1, Vector3& row2)
				{
					row0 = Vector3(m.v[0].x, m.v[0].y, m.v[0].z);
					row1 = Vector3(m.v[1].x, m.v[1].y, m.v[1].z);
					row2 = Vector3(m.v[2].x, m.v[2].y, m.v[2].z);
				}

				/** 基底ベクトル3本と位置から行列を組み立てる */
				inline void BuildMatrix(Matrix& outMat, const Vector3& row0, const Vector3& row1, const Vector3& row2, const Vector3& pos)
				{
					outMat.v[0].Set(row0.x, row0.y, row0.z, 0.0f);
					outMat.v[1].Set(row1.x, row1.y, row1.z, 0.0f);
					outMat.v[2].Set(row2.x, row2.y, row2.z, 0.0f);
					outMat.v[3].Set(pos.x, pos.y, pos.z, 1.0f);
				}
			}

			bool SolveTwoBoneLegIK(nsK2EngineLow::Skeleton* skeleton, const LegIKChain& chain, const Vector3& targetWorldPos)
			{
				if (!skeleton) return false;
				if (chain.hipBoneNo < 0 || chain.kneeBoneNo < 0 || chain.footBoneNo < 0) return false;

				nsK2EngineLow::Bone* hipBone = skeleton->GetBone(chain.hipBoneNo);
				nsK2EngineLow::Bone* kneeBone = skeleton->GetBone(chain.kneeBoneNo);
				nsK2EngineLow::Bone* footBone = skeleton->GetBone(chain.footBoneNo);
				if (!hipBone || !kneeBone || !footBone) return false;

				const Matrix& hipWorld = hipBone->GetWorldMatrix();
				const Matrix& kneeWorld = kneeBone->GetWorldMatrix();
				const Matrix& footWorld = footBone->GetWorldMatrix();

				Vector3 hipPos = GetTranslation(hipWorld);
				Vector3 kneePos = GetTranslation(kneeWorld);
				Vector3 footPos = GetTranslation(footWorld);

				// 現姿勢からボーン長を算出（毎フレーム計算するのでスケールアニメにも追従する）
				float L1 = (kneePos - hipPos).Length();
				float L2 = (footPos - kneePos).Length();
				if (L1 < FLT_EPSILON || L2 < FLT_EPSILON) return false;

				// ターゲットまでの距離を算出し、伸びきり・めり込みすぎを防ぐためクランプ
				Vector3 toTarget = targetWorldPos - hipPos;
				float D = toTarget.Length();
				if (D < FLT_EPSILON) return false; // ターゲットが股関節と重なる異常値

				const float epsilon = 0.5f; // ※プロジェクトの単位系（cm/m）に合わせて調整してください
				float maxD = (L1 + L2) * chain.maxReachRatio;
				float minD = fabsf(L1 - L2) + epsilon;
				if (maxD < minD) maxD = minD + epsilon;

				if (D > maxD) D = maxD;
				if (D < minD) D = minD;

				Vector3 dirToTarget = toTarget;
				dirToTarget.Normalize();
				Vector3 clampedTargetPos = hipPos + dirToTarget * D;

				// 余弦定理で股関節側の角度を求める
				float cosHipAngle = (L1 * L1 + D * D - L2 * L2) / (2.0f * L1 * D);
				float hipAngle = ClampedAcos(cosHipAngle);

				// 膝を曲げる方向（ポールベクトル）をワールド空間へ変換
				Vector3 hipRow0, hipRow1, hipRow2;
				GetBasisRows(hipWorld, hipRow0, hipRow1, hipRow2);
				Vector3 poleDir = hipRow0 * chain.poleHintLocal.x + hipRow1 * chain.poleHintLocal.y + hipRow2 * chain.poleHintLocal.z;
				if (poleDir.LengthSq() < FLT_EPSILON) {
					poleDir = Vector3(0.0f, -1.0f, 0.0f); // フォールバック
				}
				poleDir.Normalize();

				Vector3 bendAxis = CrossProduct(dirToTarget, poleDir);
				if (bendAxis.LengthSq() < FLT_EPSILON) {
					// dirToTargetとpoleDirがほぼ平行な場合のフォールバック
					bendAxis = CrossProduct(dirToTarget, Vector3(0.0f, 0.0f, 1.0f));
					if (bendAxis.LengthSq() < FLT_EPSILON) {
						bendAxis = Vector3(1.0f, 0.0f, 0.0f);
					}
				}
				bendAxis.Normalize();

				Vector3 hipToKneeDir = RotateAroundAxis(dirToTarget, bendAxis, hipAngle * chain.bendSign);
				Vector3 newKneePos = hipPos + hipToKneeDir * L1;
				Vector3 newFootPos = clampedTargetPos; // L1,L2,Dの関係で自動的にL2離れた位置になる

				// ---- 股関節：現在の「腰→膝」方向 から 新しい方向 へ最小回転 ----
				Vector3 newHipRow0, newHipRow1, newHipRow2;
				GetBasisRows(hipWorld, newHipRow0, newHipRow1, newHipRow2);
				RotateBasisTo(newHipRow0, newHipRow1, newHipRow2, kneePos - hipPos, hipToKneeDir);

				Matrix newHipWorld;
				BuildMatrix(newHipWorld, newHipRow0, newHipRow1, newHipRow2, hipPos);

				// ---- 膝：現在の「膝→足首」方向 から 新しい方向 へ最小回転 ----
				Vector3 newKneeRow0, newKneeRow1, newKneeRow2;
				GetBasisRows(kneeWorld, newKneeRow0, newKneeRow1, newKneeRow2);
				RotateBasisTo(newKneeRow0, newKneeRow1, newKneeRow2, footPos - kneePos, newFootPos - newKneePos);

				Matrix newKneeWorld;
				BuildMatrix(newKneeWorld, newKneeRow0, newKneeRow1, newKneeRow2, newKneePos);

				// ---- 足首：今回は回転させず位置のみ更新 ----
				// （地形の傾きに合わせて足首も傾けたい場合は、ここで法線ベースに row を再構築する）
				Matrix newFootWorld = footWorld;
				newFootWorld.v[3].Set(newFootPos.x, newFootPos.y, newFootPos.z, 1.0f);

				// Skeletonへ反映（ワールド行列 & スキニング用行列の両方）
				hipBone->SetWorldMatrix(newHipWorld);
				kneeBone->SetWorldMatrix(newKneeWorld);
				footBone->SetWorldMatrix(newFootWorld);

				Matrix* skinMatrices = skeleton->GetBoneMatricesTopAddress();
				skinMatrices[chain.hipBoneNo] = hipBone->GetInvBindPoseMatrix() * newHipWorld;
				skinMatrices[chain.kneeBoneNo] = kneeBone->GetInvBindPoseMatrix() * newKneeWorld;
				skinMatrices[chain.footBoneNo] = footBone->GetInvBindPoseMatrix() * newFootWorld;

				Vector3 hipRotAxis;
				float hipRotAngle = 0.0f;
				if (CalcMinimalRotation(kneePos - hipPos, hipToKneeDir, hipRotAxis, hipRotAngle))
				{
					for (int midNo : chain.hipToKneeMidBoneNos)
					{
						nsK2EngineLow::Bone* midBone = skeleton->GetBone(midNo);
						if (!midBone) continue;

						const Matrix& midWorld = midBone->GetWorldMatrix();
						Vector3 midPos = GetTranslation(midWorld);
						Vector3 midRow0, midRow1, midRow2;
						GetBasisRows(midWorld, midRow0, midRow1, midRow2);

						RotateRigidAroundPivot(midPos, midRow0, midRow1, midRow2, hipPos, hipPos, hipRotAxis, hipRotAngle);

						Matrix newMidWorld;
						BuildMatrix(newMidWorld, midRow0, midRow1, midRow2, midPos);
						midBone->SetWorldMatrix(newMidWorld);
						skinMatrices[midNo] = midBone->GetInvBindPoseMatrix() * newMidWorld;
					}
				}

				// ---- 膝～足首の間のツイスト補助ボーンを、膝の回転・移動差分にそのまま追従させる ----
				// （膝は位置も回転も変わるので、回転の中心はkneePos→newKneePosへ移す）
				Vector3 kneeRotAxis;
				float kneeRotAngle = 0.0f;
				bool kneeRotated = CalcMinimalRotation(footPos - kneePos, newFootPos - newKneePos, kneeRotAxis, kneeRotAngle);
				for (int midNo : chain.kneeToFootMidBoneNos)
				{
					nsK2EngineLow::Bone* midBone = skeleton->GetBone(midNo);
					if (!midBone) continue;

					const Matrix& midWorld = midBone->GetWorldMatrix();
					Vector3 midPos = GetTranslation(midWorld);
					Vector3 midRow0, midRow1, midRow2;
					GetBasisRows(midWorld, midRow0, midRow1, midRow2);

					if (kneeRotated)
					{
						RotateRigidAroundPivot(midPos, midRow0, midRow1, midRow2, kneePos, newKneePos, kneeRotAxis, kneeRotAngle);
					}
					else
					{
						// 回転はほぼ無いが、膝の移動分だけは反映しておく
						midPos = midPos - kneePos + newKneePos;
					}

					Matrix newMidWorld;
					BuildMatrix(newMidWorld, midRow0, midRow1, midRow2, midPos);
					midBone->SetWorldMatrix(newMidWorld);
					skinMatrices[midNo] = midBone->GetInvBindPoseMatrix() * newMidWorld;
				}


				Vector3 footDeltaPos = newFootPos - footPos;
				for (int childNo : chain.footChildBoneNos)
				{
					nsK2EngineLow::Bone* childBone = skeleton->GetBone(childNo);
					if (!childBone) continue;

					const Matrix& childWorld = childBone->GetWorldMatrix();
					Vector3 childRow0, childRow1, childRow2;
					GetBasisRows(childWorld, childRow0, childRow1, childRow2);
					Vector3 newChildPos = GetTranslation(childWorld) + footDeltaPos;

					Matrix newChildWorld;
					BuildMatrix(newChildWorld, childRow0, childRow1, childRow2, newChildPos);

					childBone->SetWorldMatrix(newChildWorld);
					skinMatrices[childNo] = childBone->GetInvBindPoseMatrix() * newChildWorld;
				}

				return true;
			}
		}
	}
}
