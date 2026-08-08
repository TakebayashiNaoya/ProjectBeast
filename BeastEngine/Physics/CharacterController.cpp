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
			bool isGround = false;       // 立てる床か
			bool isSteepSlope = false;   // 滑り落ちる急斜面か
			Vector3 hitPos;
			Vector3 startPos;
			Vector3 hitNormal;
			btCollisionObject* me = nullptr;
			float closestFraction = 1.0f; // dist を削除してこれに統一

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 1.0f;
				}

				Vector3 hitNormalTmp;
				if (normalInWorldSpace) {
					hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				}
				else {
					btVector3 normalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;
					hitNormalTmp = *(Vector3*)&normalWorld;
				}

				// acosfのNaNエラーを防ぐためのクランプ処理
				float dotY = hitNormalTmp.y;
				if (dotY > 1.0f) dotY = 1.0f;
				if (dotY < -1.0f) dotY = -1.0f;
				float angle = acosf(dotY);

				// 約85度以上の完全な壁は、落下判定では無視する
				if (fabsf(angle) >= Math::PI * 0.47f) {
					return 1.0f;
				}

				if (convexResult.m_hitFraction < closestFraction) {
					isHit = true;
					closestFraction = convexResult.m_hitFraction;
					hitPos = *(Vector3*)&convexResult.m_hitPointLocal;
					hitNormal = hitNormalTmp;

					// 約63度未満なら立てる床、それ以上なら急斜面
					if (fabsf(angle) < Math::PI * 0.35f) {
						isGround = true;
						isSteepSlope = false;
					}
					else {
						isGround = false;
						isSteepSlope = true;
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
			float closestFraction = 1.0f;

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
				if (convexResult.m_hitCollisionObject == me || convexResult.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) {
					return 1.0f;
				}

				Vector3 hitNormalTmp;
				if (normalInWorldSpace) {
					hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				}
				else {
					btVector3 normalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;
					hitNormalTmp = *(Vector3*)&normalWorld;
				}

				float angle = fabsf(acosf(hitNormalTmp.y));

				// 54度以上の傾斜を壁とみなす
				if (angle >= Math::PI * 0.3f) {
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
					return 1.0f;
				}

				Vector3 hitNormalTmp;
				if (normalInWorldSpace) {
					hitNormalTmp = *(Vector3*)&convexResult.m_hitNormalLocal;
				}
				else {
					btVector3 normalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;
					hitNormalTmp = *(Vector3*)&normalWorld;
				}

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
				return 1.0f;
			}
		};




		/************************************************/


		CharacterController::CharacterController()
			: m_position(Vector3::Zero)
			, m_prevPosition(Vector3::Zero)
			, m_groundNormal(Vector3::Up)
			, m_groundHeight(-FLT_MAX)
			, m_verticalVelocity(0.0f)
			, m_gravity(0.0f)
			, m_radius(0.0f)
			, m_height(0.0f)
			, m_seaLevel(-FLT_MAX)
			, m_isInited(false)
			, m_isJump(false)
			, m_isOnGround(true)
			, m_isRequestTeleport(false)
			, m_isGroundInfoValid(false)
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
							// 1. 壁のXZ平面上の法線を取得
							Vector3 hitNormalXZ = callback.hitNormal;
							hitNormalXZ.y = 0.0f;
							if (hitNormalXZ.LengthSq() > FLT_EPSILON) {
								hitNormalXZ.Normalize();
							}
							else {
								hitNormalXZ = Vector3(0, 0, 0);
							}

							// 2. カプセルが壁に接触した瞬間の「安全な中心座標」を逆算
							Vector3 dir = end - start;
							Vector3 hitCenter = start + dir * callback.closestFraction;

							// 連続でヒットしてスタックするのを防ぐため、法線方向にわずかに（0.001f）押し返す
							hitCenter += hitNormalXZ * 0.001f;

							// 3. 行きたかった残りの移動ベクトルを計算
							Vector3 remainingMove = end - hitCenter;

							// 4. 壁の法線方向の移動成分を打ち消す（壁に沿って滑らせるベクトル計算）
							float dot = remainingMove.Dot(hitNormalXZ);
							if (dot < 0.0f) {
								remainingMove -= hitNormalXZ * dot;
							}

							// 5. 次の目標座標を更新
							intendedXZPos = hitCenter + remainingMove;
							intendedXZPos.y = m_position.y;

							// 6. 次のループの開始地点を「壁の表面」ではなく「安全な中心座標」に設定（★一番の修正点）
							currentIterPos = hitCenter;
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

					// 上昇中は足元の地面情報を更新しない（前フレームの値を残さない）
					m_isGroundInfoValid = false;
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

					// カプセルのスイープは「そこに立てるか」の判定には正しいが、斜面では
					// カプセルが側面で接地するため、接地Yは真下の地面より radius*(1/cosθ-1)
					// だけ高い位置になる。見た目を地面に合わせたい側（脚IKなど）のために、
					// キャラのXZから真下へ細いレイを飛ばして「本当の地面の高さと法線」も取っておく。
					// ※物理挙動そのものは従来どおりカプセル基準のままで変えていない。
					{
						// start/end はカプセル「中心」の移動区間なので、レイにはそのまま使えない。
						// レイはキャラの足元(m_position)基準で撃つ。
						// 下方向は斜面での想定浮き量（63度で radius*1.2、尾根ではもう少し）を
						// カバーできる長さが必要なので radius*2 を確保する。
						const float rayDownReach = m_radius * 2.0f + checkDist;
						const Vector3 rayStart(m_position.x, m_position.y + stepOffset, m_position.z);
						const Vector3 rayEnd(m_position.x, m_position.y - rayDownReach, m_position.z);

						const btCollisionObject* me = m_rigidBody.GetBody();
						RaycastHit rayHit;
						const bool isRayHit = PhysicsWorld::Get().Raycast(
							rayStart, rayEnd, rayHit, ALL_COLLISION_ATTRIBUTE_MASK,
							[me](const btCollisionObject& obj) {
								if (&obj == me) return false;
								if (obj.getInternalType() == btCollisionObject::CO_GHOST_OBJECT) return false;
								return true;
							});

						if (isRayHit) {
							m_groundHeight = rayHit.point.y;
							m_groundNormal = rayHit.normal;
							// 裏面を拾った場合に法線が下を向くことがあるので反転しておく
							if (m_groundNormal.y < 0.0f) {
								m_groundNormal.x = -m_groundNormal.x;
								m_groundNormal.y = -m_groundNormal.y;
								m_groundNormal.z = -m_groundNormal.z;
							}
							m_isGroundInfoValid = true;
						}
						else {
							m_isGroundInfoValid = false;
						}
					}

					if (callback.isHit) {
						// ぶつかった地点のY座標を計算
						float hitCenterY = start.y - totalSweepDist * callback.closestFraction;
						float newY = hitCenterY - (m_height * 0.5f + m_radius);

						if (callback.isGround) {
							if (newY >= m_seaLevel) {
								// 立てる床の場合：着地
								m_position.y = newY;
								m_isOnGround = true;
								m_isJump = false;
								m_verticalVelocity = 0.0f;
							}
							else {
								// 着地点が海面より下の場合は水中地形とみなして接地しない
								m_isOnGround = false;
								m_position.y -= downAmount;
								if (m_position.y < m_seaLevel) {
									m_position.y = m_seaLevel;
									m_verticalVelocity = 0.0f;
								}
							}
						}
						else if (callback.isSteepSlope) {
							// 急斜面の場合：接地判定にせず、法線の外側に滑り落とす
							m_isOnGround = false;

							// 坂のXZ方向の法線ベクトルを計算
							Vector3 normalXZ(callback.hitNormal.x, 0.0f, callback.hitNormal.z);
							if (normalXZ.LengthSq() > FLT_EPSILON) {
								normalXZ.Normalize();
							}

							// 急な坂を「登る」のを防ぎつつ、ガタつきをなくす
							// ぶつかった地点(newY)が元の高さより高い＝足元より高い斜面に突っ込んで登ろうとしている
							if (newY > m_prevPosition.y + 0.01f) {
								// 現在のフレームでのXZ移動ベクトル
								Vector3 currentXZMove(m_position.x - m_prevPosition.x, 0.0f, m_position.z - m_prevPosition.z);

								// 内積(Dot)を使って、坂の法線と逆向き（めり込む方向）にどれくらい進んだかを計算
								float dot = currentXZMove.Dot(normalXZ);

								// 坂に向かって進んでいる場合のみ、その成分を正確に打ち消す
								if (dot < 0.0f) {
									m_position.x -= normalXZ.x * dot;
									m_position.z -= normalXZ.z * dot;
								}
								// 高さは上げず、そのまま重力に従って落ちる
								m_position.y = m_prevPosition.y;
							}
							else {
								// 上から落ちてきて斜面にぶつかった場合は、めり込みを防ぐため高さを合わせる
								m_position.y = newY;
							}

							// 重力で落下速度が無限に加速し続けるのを防ぐ
							const float MAX_SLIDE_SPEED = 100.0f;
							if (m_verticalVelocity < -MAX_SLIDE_SPEED) {
								m_verticalVelocity = -MAX_SLIDE_SPEED;
							}

							// 制限をかけた速度でXZ方向の押し出し量（滑り落ちる量）を計算
							if (normalXZ.LengthSq() > FLT_EPSILON) {
								float slideAmount = fabsf(m_verticalVelocity * deltaTime);
								float slopeFactor = 1.0f - callback.hitNormal.y;

								// 落下速度に応じて斜面を滑り落ちるようにXZ方向に押し出す
								m_position.x += normalXZ.x * slideAmount * slopeFactor;
								m_position.z += normalXZ.z * slideAmount * slopeFactor;
							}

							// 急斜面に当たっていても、海面より下に潜らないようにする
							if (m_position.y < m_seaLevel) {
								m_position.y = m_seaLevel;
								m_verticalVelocity = 0.0f;
							}
						}
					}
					else {
						// ★ここが外に出ます！：何にもぶつからなかった場合（空中）
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