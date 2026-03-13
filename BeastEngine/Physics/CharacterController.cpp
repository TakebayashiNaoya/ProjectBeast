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

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 1.0f;
				}

				Vector3 hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				float angle = acosf(hitNormalTmp.y); // 上(0,1,0)との角度

				// 地面判定 (法線が上を向いている)
				if (fabsf(angle) < Math::PI * 0.3f) {
					isHit = true;
					Vector3 hitPosTmp = *(Vector3*)&convexResult.m_hitPointLocal;
					Vector3 vDist = hitPosTmp - startPos;
					float distTmp = vDist.Length();
					if (dist > distTmp) {
						hitPos = hitPosTmp;
						hitNormal = hitNormalTmp;
						dist = distTmp;
					}
				}
				return 0.0f;
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
				// 壁判定 (法線が横を向いている = 上との角度が大きい)
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
				// 下方向と法線のなす角度をチェック、あるいは単純に y がマイナス（下向き）か
				if (hitNormalTmp.y < -0.5f) { // 法線が下を向いている＝天井
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
			, m_isInited(false)
			, m_isJump(false)
			, m_isOnGround(true)
			, m_isRequestTeleport(false)
		{
		}


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

			//m_rigidBody.GetBody()->setUserIndex(enCollisionAttr_Character);
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
				// テレポート処理
				// 物理演算をスキップして座標を強制適用

				m_position = targetPosition;

				// 内部物理状態のリセット
				m_verticalVelocity = 0.0f; // 落下の勢いなどを消す
				m_isOnGround = true;       // 安全のため一旦接地扱いにする（あるいは空中扱いにしたい場合はfalse）
				m_isJump = false;

				// フラグを消費して終了
				m_isRequestTeleport = false;
			}
			else {
				// 通常の物理移動処理

				// 重力の適用
				m_verticalVelocity += m_gravity * deltaTime;

				Vector3 nextPosition = m_position;
				Vector3 intendedXZPos = targetPosition;
				intendedXZPos.y = m_position.y; // Yは重力計算に任せるためここでは維持

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

						// SweepTest設定
						Vector3 posTmp = currentIterPos;
						posTmp.y += m_height * 0.5f + m_radius + m_height * 0.1f;

						Vector3 start(posTmp.x, posTmp.y, posTmp.z);
						Vector3 end(intendedXZPos.x, posTmp.y, intendedXZPos.z);

						SweepResultWall callback;
						callback.me = m_rigidBody.GetBody();
						callback.startPos = posTmp;

						//PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);

						if (callback.isHit) {
							// 壁衝突：押し戻し計算
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

					SweepResultCeiling callback; // ※前回の定義を参照
					callback.me = m_rigidBody.GetBody();
					callback.startPos = m_position;
					callback.startPos.y = checkY;

					if ((start - end).LengthSq() >= 0.01f) {
						//PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);
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
					float checkDist = (m_isOnGround) ? 0.5f : downAmount + 0.1f;
					float checkY = m_position.y + m_height * 0.5f + m_radius;

					Vector3 start(m_position.x, checkY, m_position.z);
					Vector3 end(m_position.x, checkY - checkDist, m_position.z);

					SweepResultGround callback;
					callback.me = m_rigidBody.GetBody();
					callback.startPos = Vector3(m_position.x, checkY, m_position.z);

					//PhysicsWorld::Get().ConvexSweepTest(m_collider, start, end, callback);

					if (callback.isHit) {
						m_isOnGround = true;
						m_isJump = false;
						m_verticalVelocity = 0.0f;
						m_position.y = callback.hitPos.y;
					}
					else {
						m_isOnGround = false;
						m_position.y -= downAmount;
					}
				}
			}

			// 剛体（Collider）の位置を更新
			// テレポート時も通常時も、最終的な m_position を反映させる
			btRigidBody* btBody = m_rigidBody.GetBody();
			btBody->setActivationState(DISABLE_DEACTIVATION);
			btTransform& trans = btBody->getWorldTransform();
			trans.setOrigin(btVector3(m_position.x, m_position.y + m_height * 0.5f + m_radius, m_position.z));

			return m_position;
		}


		void CharacterController::RemoveRigidBoby()
		{
			//PhysicsWorld::Get().RemoveRigidBody(m_rigidBody);
		}


		void CharacterController::Bounce(const float power)
		{
			/** 地面にいるかどうかの判定を無視して、強制的に上方向の速度を上書きする */
			m_verticalVelocity = power;
			m_isOnGround = false;
		}
	}
}