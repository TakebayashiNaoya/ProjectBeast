/**
 * @file BoxCollider.h
 * @brief ボックスコライダー
 * @author 竹林尚哉
 */
#pragma once
#include "ICollider.h"


namespace nsBeastEngine
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
		 * @brief コライダーを取得
		 */
		btCollisionShape* GetBody() const override
		{
			return shape.get();
		}


	private:
		/** 当たり判定用ボックス */
		std::unique_ptr<btBoxShape>	shape;
	};
}