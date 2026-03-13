/*!
* @brief	球体コライダー。
*/
#include "BeastEnginePreCompile.h"
#include "Physics.h"
#include "SphereCollider.h"

namespace nsBeastEngine
{
	namespace nsCollision
	{
		void SphereCollider::Create(const float radius)
		{
			m_shape = std::make_unique<btSphereShape>(radius);
		}
	}
}
