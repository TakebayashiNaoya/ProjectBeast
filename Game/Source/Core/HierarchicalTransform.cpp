/**
 * HierarchicalTransform.cpp
 * 階層型トランスフォーム実装
 */
#include "stdafx.h"
#include "HierarchicalTransform.h"


namespace app
{
	namespace core
	{
		HierarchicalTransform::HierarchicalTransform()
			: m_worldMatrix(Matrix::Identity)
			, m_parent(nullptr)
		{
			m_children.clear();

			/** m_localTransformの初期化 */
			m_localTransform.m_position = Vector3::Zero;
			m_localTransform.m_rotation = Quaternion::Identity;
			m_localTransform.m_scale = Vector3::One;

			/** m_worldTransformの初期化 */
			m_worldTransform.m_position = Vector3::Zero;
			m_worldTransform.m_rotation = Quaternion::Identity;
			m_worldTransform.m_scale = Vector3::One;
		}


		HierarchicalTransform::~HierarchicalTransform()
		{
			if (m_parent) {
				m_parent->RemoveChild(this);
			}
			RemoveAllChildren();
		}


		void HierarchicalTransform::UpdateTransform()
		{
			if (m_parent)
			{
				/** 自分のローカル座標（親からどれだけズレているか）を行列化 */
				Matrix localPos;
				localPos.MakeTranslation(m_localTransform.m_position);

				/** 親のワールド行列と掛け合わせてワールド座標を算出 */
				Matrix pos;
				pos.Multiply(localPos, m_parent->m_worldMatrix);

				/** 平行移動成分を抽出し、自分のワールド座標に入れる */
				m_worldTransform.m_position.x = pos.m[3][0];
				m_worldTransform.m_position.y = pos.m[3][1];
				m_worldTransform.m_position.z = pos.m[3][2];

				/** 自分のスケールと親のスケールを掛け合わせて、自分のワールドスケールを算出 */
				m_worldTransform.m_scale.x = m_localTransform.m_scale.x * m_parent->m_worldTransform.m_scale.x;
				m_worldTransform.m_scale.y = m_localTransform.m_scale.y * m_parent->m_worldTransform.m_scale.y;
				m_worldTransform.m_scale.z = m_localTransform.m_scale.z * m_parent->m_worldTransform.m_scale.z;

				/** 自分の回転と親の回転を掛け合わせて、自分のワールド回転を算出 */
				m_worldTransform.m_rotation = m_parent->m_worldTransform.m_rotation * m_localTransform.m_rotation;
			}
			else
			{
				/** 親がいない場合は、ローカルの値をそのままコピー */
				m_worldTransform.m_position = m_localTransform.m_position;
				m_worldTransform.m_scale = m_localTransform.m_scale;
				m_worldTransform.m_rotation = m_localTransform.m_rotation;
			}

			/** ここまで計算したワールド座標・ワールド拡大縮小・ワールド回転を1つのワールド行列にまとめる */
			UpdateWorldMatrix();
		}


		void HierarchicalTransform::UpdateWorldMatrix()
		{
			Matrix scal, rot, pos, world;
			/** 拡大縮小行列を作成 */
			scal.MakeScaling(m_worldTransform.m_scale);
			/** 回転行列を作成 */
			rot.MakeRotationFromQuaternion(m_worldTransform.m_rotation);
			/** 平行移動行列を作成 */
			pos.MakeTranslation(m_worldTransform.m_position);
			/** 拡大縮小→回転→平行移動の順で行列を掛け合わせる */
			world.Multiply(scal, rot);
			m_worldMatrix.Multiply(world, pos);

			/** 自分の子も更新 */
			for (HierarchicalTransform* child : m_children) {
				child->UpdateTransform();
			}
		}


		void HierarchicalTransform::AddChild(HierarchicalTransform* childTransform)
		{
			if (HasChild(childTransform)) {
				return;
			}
			m_children.push_back(childTransform);
		}


		void HierarchicalTransform::RemoveChild(HierarchicalTransform* childTransform)
		{
			auto it = m_children.begin();
			while (it != m_children.end())
			{
				if ((*it) == childTransform)
				{
					/** 子の親をクリア */
					(*it)->m_parent = nullptr;
					/** 子リストから削除 */
					it = m_children.erase(it);
					return;
				}
				else
				{
					++it;
				}
			}
		}


		void HierarchicalTransform::RemoveAllChildren()
		{
			/** 各子の親をクリアする */
			for (auto* child : m_children) {
				if (child) {
					child->m_parent = nullptr;
				}
			}
			m_children.clear();
		}


		bool HierarchicalTransform::HasChild(HierarchicalTransform* childTransform) const
		{
			for (auto* child : m_children) {
				if (child == childTransform) {
					return true;
				}
			}
			return false;
		}


		void HierarchicalTransform::SetParent(HierarchicalTransform* parentTransform)
		{
			/** すでに親がいるなら解除 */
			if (m_parent) {
				RemoveParent();
			}

			m_parent = parentTransform;

			/** 新しい親に自分を追加してもらう */
			if (m_parent) {
				m_parent->AddChild(this);
			}
		}


		void HierarchicalTransform::RemoveParent()
		{
			/** 親がいるなら、親の子供リストから自分を外してもらう */
			if (m_parent) {
				m_parent->RemoveChild(this);
				m_parent = nullptr;
			}
		}
	}
}