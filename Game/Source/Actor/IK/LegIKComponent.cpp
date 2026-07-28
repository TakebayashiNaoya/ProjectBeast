/**
 * @file LegIKComponent.cpp
 * @brief キャラクター1体分の脚IKをまとめて管理するコンポーネントの実装
 */
#include "stdafx.h" // ※プロジェクトのプリコンパイル済みヘッダ名に合わせて調整してください
#include "../../../../k2EngineLow/graphics/Skeleton.h"
#include "LegIKComponent.h"
#include "Source/Actor/Stage/TerrainObject.h" // ※実際のインクルードパスに合わせて調整してください

namespace app
{
	namespace actor
	{
		namespace ik
		{
			bool LegIKComponent::AddLegByBoneNames(
				nsK2EngineLow::Skeleton* skeleton,
				const wchar_t* hipBoneName,
				const wchar_t* kneeBoneName,
				const wchar_t* footBoneName,
				const Vector3& poleHintLocal,
				float bendSign,
				float footGroundOffset,
				const std::vector<const wchar_t*>& footChildBoneNames,
				const std::vector<const wchar_t*>& hipToKneeMidBoneNames,
				const std::vector<const wchar_t*>& kneeToFootMidBoneNames)
			{
				if (!skeleton) return false;

				int hipNo = skeleton->FindBoneID(hipBoneName);
				int kneeNo = skeleton->FindBoneID(kneeBoneName);
				int footNo = skeleton->FindBoneID(footBoneName);

				if (hipNo < 0 || kneeNo < 0 || footNo < 0) {
					return false;
				}

				LegIKChain chain;
				chain.hipBoneNo = hipNo;
				chain.kneeBoneNo = kneeNo;
				chain.footBoneNo = footNo;
				chain.poleHintLocal = poleHintLocal;
				chain.bendSign = bendSign;
				chain.footGroundOffset = footGroundOffset;

				for (const wchar_t* name : footChildBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.footChildBoneNos.push_back(boneNo); }
				}
				for (const wchar_t* name : hipToKneeMidBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.hipToKneeMidBoneNos.push_back(boneNo); }
				}
				for (const wchar_t* name : kneeToFootMidBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.kneeToFootMidBoneNos.push_back(boneNo); }
				}

				m_legs.push_back(chain);
				return true;
			}

			void LegIKComponent::Update(nsK2EngineLow::Skeleton* skeleton)
			{
				if (!skeleton || !m_terrain) return;

				for (auto& leg : m_legs)
				{
					nsK2EngineLow::Bone* footBone = skeleton->GetBone(leg.footBoneNo);
					if (!footBone) continue;

					const Matrix& footWorld = footBone->GetWorldMatrix();
					Vector3 currentFootWorldPos(footWorld.v[3].x, footWorld.v[3].y, footWorld.v[3].z);

					// XZはアニメーションのまま、Yだけ地形の高さに合わせる。
					// （足跡デカールで実装済みのGetHeightAtをそのまま利用できます）
					// ※GetHeightAtはVector3を1つ受け取る仕様（Y成分は内部で無視される）
					float terrainHeight = m_terrain->GetHeightAt(currentFootWorldPos);

					Vector3 target = currentFootWorldPos;
					target.y = terrainHeight + leg.footGroundOffset;

					SolveTwoBoneLegIK(skeleton, leg, target);
				}
			}
		}
	}
}
