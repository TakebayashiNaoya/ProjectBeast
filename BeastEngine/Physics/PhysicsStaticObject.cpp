/*!
* @brief	静的オブジェクト。
*/



#include "BeastEnginePreCompile.h"
#include "physics/PhysicsStaticObject.h"

namespace nsBeastEngine {
	PhysicsStaticObject::PhysicsStaticObject()
	{
	}
	PhysicsStaticObject::~PhysicsStaticObject()
	{
	}
	void PhysicsStaticObject::CreateFromModel(Model& model, const Matrix& worldMatrix, const float restitution)
	{
		m_meshCollider.CreateFromModel(model, worldMatrix);
		RigidBodyInitData rbInfo;
		rbInfo.collider = &m_meshCollider;
		rbInfo.mass = 0.0f;
		rbInfo.restitution = restitution;
		m_rigidBody.Init(rbInfo);
	}
}
