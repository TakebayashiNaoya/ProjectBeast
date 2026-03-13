
#include "BeastEnginePreCompile.h"
#include "RigidBody.h"

namespace nsBeastEngine
{
	namespace nsCollision
	{
		void RigidBody::Init(RigidBodyInitData& initData)
		{
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(initData.pos.x, initData.pos.y, initData.pos.z));
			transform.setRotation(btQuaternion(initData.rot.x, initData.rot.y, initData.rot.z, initData.rot.w));

			m_myMotionState = std::make_unique<btDefaultMotionState>();
			m_myMotionState->setWorldTransform(transform);
			btVector3 btLocalInteria(0.0f, 0.0f, 0.0f);
			btCollisionShape* m_shape = initData.collider->GetBody();
			if (initData.mass != 0.0f) {
				m_shape->calculateLocalInertia(initData.mass, btLocalInteria);
			}

			btRigidBody::btRigidBodyConstructionInfo btRbInfo(
				initData.mass,
				m_myMotionState.get(),
				m_shape,
				btLocalInteria
			);
			//反発力を設定。
			btRbInfo.m_restitution = initData.restitution;
			m_rigidBody = std::make_unique<btRigidBody>(btRbInfo);

			//PhysicsWorld::Get().AddRigidBody(*this);
		}
		RigidBody::~RigidBody()
		{
			Release();
		}

		void RigidBody::Release()
		{
			if (m_rigidBody) {
				//PhysicsWorld::Get().RemoveRigidBody(*this);
			}
		}
	}
}