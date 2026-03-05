/**
 * @file CapsuleCollider.h
 * @brief カプセルコライダー
 * @author 竹林尚哉
 */
#pragma once
#include "ICollider.h"


namespace nsBeastEngine
{
	/**
	 * @brief カプセルコライダー
	 */
	class CapsuleCollider : public ICollider
	{
	public:
		/**
		 * @brief 生成
		 * @param radius 半径
		 * @param height 高さ
		 */
		void Init(float radius, float height)
		{
			m_shape = std::make_unique<btCapsuleShape>(radius, height);
			m_radius = radius;
			m_height = height;
		}

		/**
		 * @brief BulletPhysicsのコリジョン形状を取得
		 * @return コリジョン形状
		 */
		btCollisionShape* GetBody() const override
		{
			return m_shape.get();
		}

		/**
		 * @brief 半径を取得
		 * @return 半径
		 */
		float GetRadius() const
		{
			return m_radius;
		}

		/**
		 * @brief 高さを取得
		 * @return 高さ
		 */
		float GetHeight() const
		{
			return m_height;
		}


	private:
		/** 当たり判定用カプセル */
		std::unique_ptr<btCapsuleShape>	m_shape;

		float m_radius = 0.0f;
		float m_height = 0.0f;
	};
}