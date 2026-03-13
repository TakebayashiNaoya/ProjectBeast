/**
 * @file BoxCollider.cpp
 * @brief ボックスコライダーの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "Physics.h"
#include "BoxCollider.h"


namespace nsBeastEngine
{
	namespace nsCollision
	{
		void BoxCollider::Create(const Vector3& size)
		{
			/** BulletのbtBoxShapeは半分のサイズを指定する必要があるため、0.5倍して渡す */
			m_shape = std::make_unique<btBoxShape>(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
		}
	}
}

