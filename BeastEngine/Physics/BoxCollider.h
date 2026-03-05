/**
 * @file BoxCollider.h
 * @brief ボックスコライダー
 * @author 竹林尚哉
 */
#pragma once
#include "ICollider.h"


namespace nsBeastEngine
{
	namespace nsCollision
	{
		/**
		 * @brief ボックスコライダー。
		 */
		class BoxCollider : public ICollider
		{
		public:
			/**
			 * @brief 生成
			 */
			void Create(const Vector3& size);

			/**
			 * @brief BulletPhysicsのコリジョン形状を取得
			 * @return コリジョン形状
			 */
			btCollisionShape* GetBody() const override
			{
				return m_shape.get();
			}


		private:
			/** 当たり判定用ボックス */
			std::unique_ptr<btBoxShape>	m_shape;
		};
	}
}