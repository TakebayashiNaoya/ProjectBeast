/**
 * @file CharacterController.cpp
 * @brief キャラクターコントローラーの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "CharacterController.h"

namespace nsBeastEngine
{
	namespace nsCollision
	{
		/** 地面判定 */
		struct SweepResultGround : public btCollisionWorld::ConvexResultCallback {
			bool isHit = false;
			Vector3 hitPos;
			Vector3 startPos;
			Vector3 hitNormal;
			btCollisionObject* me = nullptr;
			float dist = FLT_MAX;
			float closestFraction = 1.0f;

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 1.0f;
				}

				Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				float angle = acosf(hitNormalTmp.y);

				if (fabsf(angle) < Math::PI * 0.35f) {
					if (convexResult.m_hitFraction < closestFraction) {
						isHit = true;
						closestFraction = convexResult.m_hitFraction;

						hitPos = *(Vector3*)&convexResult.m_hitPointLocal;
						hitNormal = hitNormalTmp;
					}
				}

				return 1.0f;
			}
		};


		/** 壁判定 */
		struct SweepResultWall : public btCollisionWorld::ConvexResultCallback {
			bool isHit = false;
			Vector3 hitPos;
			Vector3 startPos;
			Vector3 hitNormal;
			btCollisionObject* me = nullptr;
			float dist = FLT_MAX;

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 1.0f;
				}

				Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				float angle = fabsf(acosf(hitNormalTmp.y));
				if (angle >= Math::PI * 0.3f) {
					isHit = true;
					Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
					Vector3 vDist = hitPosTmp - startPos;
					vDist.y = 0.0f;
					float distTmp = vDist.Length();
					if (distTmp < dist) {
						hitPos = hitPosTmp;
						dist = distTmp;
						hitNormal = hitNormalTmp;
					}
				}
				return 0.0f;
			}
		};

		/** 天井用 */
		struct SweepResultCeiling : public btCollisionWorld::ConvexResultCallback
		{
			bool isHit = false;
			Vector3 hitPos;
			Vector3 startPos;
			btCollisionObject* me = nullptr;
			float dist = FLT_MAX;

			virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
			{
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 0.0f;
				}

				Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				if (hitNormalTmp.y < -0.5f) {
					isHit = true;
					Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
					Vector3 vDist;
					vDist.Subtract(hitPosTmp, startPos);
					float distTmp = vDist.Length();
					if (dist > distTmp) {
						hitPos = hitPosTmp;
						dist = distTmp;
					}
				}
				return 0.0f;
			}
		};




		/************************************************/


		CharacterController::CharacterController()
			: m_position(Vector3::Zero)
			, m_prevPosition(Vector3::Zero)
			, m_verticalVelocity(0.0f)
			, m_gravity(0.0f)
			, m_radius(0.0f)
			, m_height(0.0f)
			, m_seaLevel(-FLT_MAX)
			, m_isInited(false)
			, m_isJump(false)
			, m_isOnGround(true)
			, m_isRequestTeleport(false)
		{}


		CharacterController::~CharacterController()
		{
			RemoveRigidBoby();
		}


		void CharacterController::Init(float radius, float height, const Vector3& position)
		{
			m_position = position;
			m_prevPosition = position;
			m_radius = radius;
			m_height = height;
			m_collider.Init(radius, height);

			RigidBodyInitData rbInfo;
			rbInfo.collider = &m_collider;
			rbInfo.mass = 0.0f;
			m_rigidBody.Init(rbInfo);

			btTransform& trans = m_rigidBody.GetBody()->getWorldTransform();
			trans.setOrigin(btVector3(position.x, position.y + m_height * 0.5f + m_radius, position.z));

			m_rigidBody.GetBody()->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

			m_isInited = true;
		}


		void CharacterController::Jump(float jumpPower)
		{
			if (m_isOnGround) {
				m_verticalVelocity = jumpPower;
				m_isOnGround = false;
				m_isJump = true;
			}
		}


		const Vector3& CharacterController::Execute(const Vector3& targetPosition, float deltaTime)
		{
			// 前フレームの座標を保存
			m_prevPosition = m_position;

			// テレポートリクエストの確認
			if (m_isRequestTeleport) {
				m_position = targetPosition;

				m_verticalVelocity = 0.0f;
				m_isOnGround = true;
				m_isJump = false;

				m_isRequestTeleport = false;
			}
			else {
				// 重力の適用
				m_verticalVelocity += m_gravity * deltaTime;

				Vector3 nextPosition = m_position;
				Vector3 intendedXZPos = targetPosition;
				intendedXZPos.y = m_position.y;

				// XZ平面（壁）の移動解決
				{
					int loopCount = 0;
					Vector3 currentIterPos = m_position;

					while (true) {
						Vector3 moveDir = intendedXZPos - currentIterPos;
						moveDir.y = 0.0f;
						if (moveDir.Length() < FLT_EPSILON) {
							nextPosition.x = intendedXZPos.x;
							nextPosition.z = intendedXZPos.z;
							break;
						}

						Vector3 posTmp = currentIterPos;
						posTmp.y += m_height * 0.5f + m_radius + m_height * 0.1f;

						Vector3 start(posTmp.x, posTmp.y, posTmp.z);
						Vector3 end(intendedXZPos.x, posTmp.y, intendedXZPos.z);

						SweepResultWall callback;
						callback.me = m_rigidBody.GetBody();
						callback.startPos = posTmp;

						PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);

						if (callback.isHit) {
							Vector3 vT0(intendedXZPos.x, 0.0f, intendedXZPos.z);
							Vector3 vT1(callback.hitPos.x, 0.0f, callback.hitPos.z);
							Vector3 vMerikomi = vT0 - vT1;

							Vector3 hitNormalXZ = callback.hitNormal;
							hitNormalXZ.y = 0.0f;
							hitNormalXZ.Normalize();

							float fT0 = hitNormalXZ.Dot(vMerikomi);
							Vector3 vOffset = hitNormalXZ;
							vOffset.Scale(-fT0 + m_radius + 0.001f);

							intendedXZPos += vOffset;
							currentIterPos = callback.hitPos;
							currentIterPos.y = m_position.y;
						}
						else {
							nextPosition.x = intendedXZPos.x;
							nextPosition.z = intendedXZPos.z;
							break;
						}
						loopCount++;
						if (loopCount >= 5) break;
					}
				}
				m_position.x = nextPosition.x;
				m_position.z = nextPosition.z;

				// Y軸（天井・床）の解決
				if (m_verticalVelocity > 0.0f) {
					// 上昇中（天井判定）
					float upAmount = m_verticalVelocity * deltaTime;
					float checkY = m_position.y + m_height * 0.5f + m_radius;

					Vector3 start(m_position.x, checkY, m_position.z);
					Vector3 end(m_position.x, checkY + upAmount, m_position.z);

					SweepResultCeiling callback;
					callback.me = m_rigidBody.GetBody();
					callback.startPos = m_position;
					callback.startPos.y = checkY;

					if ((start - end).LengthSq() >= 0.01f) {
						PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);
					}
					if (callback.isHit) {
						m_verticalVelocity = 0.0f;
						float dist = (callback.hitPos.y - checkY);
						if (dist < 0) dist = 0;
						m_position.y += dist - 0.01f;
					}
					else {
						m_position.y += upAmount;
					}
					m_isOnGround = false;
				}
				else
				{
					// 落下中（床判定）
					float downAmount = fabsf(m_verticalVelocity * deltaTime);

					float stickDist = 5.0f;
					float checkDist = (m_isOnGround) ? stickDist : downAmount + 0.1f;

					Vector3 xzMove(m_position.x - m_prevPosition.x, 0.0f, m_position.z - m_prevPosition.z);
					float moveDist = xzMove.Length();
					float maxSlopeRise = moveDist * 2.5f;
					float stepOffset = maxSlopeRise + m_radius * 2.0f;

					float totalSweepDist = stepOffset + checkDist;

					float checkY = m_position.y + m_height * 0.5f + m_radius;
					Vector3 start(m_position.x, checkY + stepOffset, m_position.z);
					Vector3 end(m_position.x, start.y - totalSweepDist, m_position.z);

					SweepResultGround callback;
					callback.me = m_rigidBody.GetBody();

					PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);

					if (callback.isHit) {
						m_isOnGround = true;
						m_isJump = false;
						m_verticalVelocity = 0.0f;

						float hitCenterY = start.y - totalSweepDist * callback.closestFraction;
						m_position.y = hitCenterY - (m_height * 0.5f + m_radius);
					}
					else {
						m_isOnGround = false;
						m_position.y -= downAmount;

						// 波面より下に潜らないようにする
						if (m_position.y < m_seaLevel)
						{
							m_position.y = m_seaLevel;
							m_verticalVelocity = 0.0f;
						}
					}
				}
			}

			// 剛体（Collider）の位置を更新
			btRigidBody* btBody = m_rigidBody.GetBody();
			btBody->setActivationState(DISABLE_DEACTIVATION);
			btTransform& trans = btBody->getWorldTransform();
			trans.setOrigin(btVector3(m_position.x, m_position.y + m_height * 0.5f + m_radius, m_position.z));

			return m_position;
		}


		void CharacterController::RemoveRigidBoby()
		{
			if (!m_isInited) return;
			PhysicsWorld::Get().RemoveRigidBody(m_rigidBody);
			m_isInited = false;
		}


		void CharacterController::Bounce(const float power)
		{
			m_verticalVelocity = power;
			m_isOnGround = false;
		}
	}
}