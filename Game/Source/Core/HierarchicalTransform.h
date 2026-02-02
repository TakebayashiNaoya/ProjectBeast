/**
 * HierarchicalTransform.h
 * 階層型トランスフォーム
 */
#pragma once
#include "Transform.h"


namespace app
{
	namespace core
	{
		class HierarchicalTransform : public Noncopyable
		{
		public:
			/** 自身のトランスフォーム */
			Transform m_localTransform;
			/** 親トランスフォームを考慮したパラメータ */
			Transform m_worldTransform;


		private:
			/** 親トランスフォームを考慮したワールド行列 */
			Matrix m_worldMatrix;
			/** 親トランスフォーム */
			HierarchicalTransform* m_parent;
			/** 子トランスフォーム群 */
			std::vector<HierarchicalTransform*> m_children;


		public:
			HierarchicalTransform();
			~HierarchicalTransform();


		public:
			/**
			 * 親がいる場合は、親のトランスフォームと自身のトランスフォームを掛け合わせ、自身のワールドトランスフォームを更新する
			 * 親がいない場合は、自身のローカルトランスフォームをそのままワールドトランスフォームにコピーする
			 */
			void UpdateTransform();

			/**
			 * 子を追加する
			 */
			void AddChild(HierarchicalTransform* child);

			/**
			 * 特定の子を削除する
			 */
			void RemoveChild(HierarchicalTransform* child);

			/**
			 * すべての子を削除する
			 */
			void RemoveAllChildren();

			/**
			 * 指定した子が自分のリストにいるか確認する
			 */
			bool HasChild(HierarchicalTransform* child) const;

			/**
			 * 親を設定する
			 */
			void SetParent(HierarchicalTransform* parent);

			/**
			 * 親を解除する
			 */
			void RemoveParent();

			/**
			 * 親がいるか
			 */
			bool HasParent() const
			{
				return m_parent != nullptr;
			}

			/**
			 * 親を解除
			 */
			void ClearParent()
			{
				m_parent = nullptr;
			}


		private:
			/** ワールド行列更新、UpdateTransformの方で呼ばれるので呼び出す必要なし */
			void UpdateWorldMatrix();
		};
	}
}