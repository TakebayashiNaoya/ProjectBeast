/**
 * @file LegIKComponent.h
 * @brief キャラクター1体分の脚IKをまとめて管理するコンポーネント
 * @details
 *  2足（ペンギン）でも4足（シロクマ）でも、脚の本数分 AddLeg / AddLegByBoneNames を
 *  呼んでおけば同じ仕組みで動きます（ペンギンなら2回、シロクマなら4回）。
 *  毎フレーム、Skeleton::Update() のあとに Update() を呼んでください。
 */
#pragma once
#include "TwoBoneLegIK.h"
#include <vector>

namespace nsK2EngineLow
{
	class Skeleton;
}

namespace app
{
	namespace actor
	{
		class TerrainObject;

		namespace ik
		{
			class LegIKComponent
			{
			public:
				/**
				 * @brief 参照する地形を設定
				 */
				void SetTerrain(TerrainObject* terrain) { m_terrain = terrain; }

				/**
				 * @brief 脚のチェーンを直接追加（ボーン番号が分かっている場合）
				 */
				void AddLeg(const LegIKChain& chain) { m_legs.push_back(chain); }

				/**
				 * @brief ボーン名から脚のチェーンを追加
				 * @return ボーンが見つからず追加できなかった場合はfalse
				 */
				bool AddLegByBoneNames(
					nsK2EngineLow::Skeleton* skeleton,
					const wchar_t* hipBoneName,
					const wchar_t* kneeBoneName,
					const wchar_t* footBoneName,
					const Vector3& poleHintLocal,
					float bendSign = 1.0f,
					float footGroundOffset = 0.0f,
					const std::vector<const wchar_t*>& footChildBoneNames = {},
					const std::vector<const wchar_t*>& hipToKneeMidBoneNames = {},
					const std::vector<const wchar_t*>& kneeToFootMidBoneNames = {});

				/**
				 * @brief 全ての脚のIKを解いて反映する
				 * @details Skeleton::Update() の後、そのフレームの描画・
				 *          足跡デカール生成（DecalManagerへの登録）より前に呼んでください。
				 *          IK後の足の位置に足跡を出したほうが見た目が合うはずです。
				 */
				void Update(nsK2EngineLow::Skeleton* skeleton);

				/** 登録済みの脚の数（デバッグ表示等に） */
				size_t GetLegCount() const { return m_legs.size(); }

			private:
				std::vector<LegIKChain> m_legs;
				TerrainObject* m_terrain = nullptr;
			};
		}
	}
}
