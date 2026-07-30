/**
 * @file Enemy.cpp
 * @brief エネミークラス
 * @author 立山
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/StageSystem.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 地形法線に沿わせる補間速度 */
			constexpr float GROUND_TILT_SLERP_SPEED = 10.0f;
			/** レイの発射点を、キャラクター座標からどれだけ上にずらすか（地形より確実に高くするため） */
			constexpr float GROUND_TILT_RAY_START_UP_OFFSET = 500.0f;
			/** 真下レイの射程距離（発射点を上げた分、長めに取っておく） */
			constexpr float GROUND_TILT_RAY_LENGTH = 1000.0f;

			AnimationData ANIMATION_DATA[] =
			{
				{ "Assets/animData/bear/idle.tka", true },
				{ "Assets/animData/bear/idle_UnderWater.tka", true },
				{ "Assets/animData/bear/walk.tka", true },
				{ "Assets/animData/bear/attack.tka", false },
				{ "Assets/animData/bear/attack_UnderWater.tka", false },
				{ "Assets/animData/bear/backWalk.tka", true },
				{ "Assets/animData/bear/run.tka", true },
				{ "Assets/animData/bear/swim.tka", true },
				{ "Assets/animData/bear/buff.tka", false },
				{ "Assets/animData/bear/damage.tka", true },
				{ "Assets/animData/bear/eat.tka", true },
				{ "Assets/animData/bear/stun.tka", true },
				{ "Assets/animData/bear/sleep.tka", true },

			};


			ModelData ENEMY_MODEL_DATA =
			{
				"Assets/modelData/whiteBear/WhiteBear.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};

		}


		Enemy::Enemy()
		{
			Init(ENEMY_MODEL_DATA);

			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();

			m_stateMachine->Setup(this);
			m_characterStateMachine = m_stateMachine.get();
		}


		void Enemy::Start()
		{
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			m_stateMachine->Update();

			CharacterBase::Update();

			if (m_modelReady && !m_legIKInited)
			{
				InitLegIK();
				m_legIKInited = true;
			}

			if (m_legIKInited)
			{
				UpdateGroundTilt();

				TerrainObject* terrain = StageSystem::GetInstance()->GetTerrain();

				m_legIK.SetTerrain(terrain);

				m_legIK.Update(&m_skeleton);
			}
		}


		void Enemy::UpdateGroundTilt()
		{
			if (!m_modelReady) return;

			/** 真下にレイを飛ばして地形法線を取得する（PenguinBase::UpdateSlideTiltと同じ） */
			const Vector3& pos = m_transform.m_position;

			TerrainObject* terrain = StageSystem::GetInstance()->GetTerrain();
			if (terrain)
			{
				float terrainHeight = terrain->GetHeightAt(pos);
				char buf[256];
				sprintf_s(buf, "EnemyPosY=%.1f TerrainHeight=%.1f Diff=%.1f\n", pos.y, terrainHeight, pos.y - terrainHeight);
				OutputDebugStringA(buf);
			}

			const Vector3 rayStart = Vector3(pos.x, pos.y + GROUND_TILT_RAY_START_UP_OFFSET, pos.z);
			const Vector3 rayEnd = Vector3(pos.x, pos.y - GROUND_TILT_RAY_LENGTH, pos.z);

			nsBeastEngine::nsCollision::RaycastHit hit;
			Vector3 groundNormal = Vector3::Up;
			bool hitSuccess = nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit);

			if (hitSuccess)
			{
				groundNormal = hit.normal;
			}

			/** 進行方向由来のY軸回転（水平の向き）はアニメーション側の回転をそのまま使う */
			Quaternion yRot = m_transform.m_rotation;

			/** Up → groundNormal への最短回転 */
			Quaternion tiltRot;
			tiltRot.SetRotation(Vector3::Up, groundNormal);

			/** 「まず地形に合わせて傾け、次に進行方向を向く」合成 */
			Quaternion targetRotation;
			targetRotation.Multiply(yRot, tiltRot);

			/** 補間して急激な回転変化を抑える */
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const float slerpFactor = min(1.0f, GROUND_TILT_SLERP_SPEED * deltaTime);
			m_groundTiltRotation.Slerp(slerpFactor, m_groundTiltRotation, targetRotation);

			// ★修正: m_modelRender.Update()を呼ぶとアニメーション再生時間が
			//         CharacterBase::Update()内の分と合わせて二重に進んでしまう（倍速の原因）。
			//         Skeleton::Update()はアニメーション時間を進めず行列を掛け合わせるだけなので、
			//         こちらを直接呼び、今フレーム分すでに計算済みのローカル行列（＝アニメのポーズ）は
			//         そのままに、ワールド行列だけを「傾いたルート行列」で再計算する。
			Vector3 rowX = Vector3::AxisX; m_groundTiltRotation.Apply(rowX); rowX = rowX * m_transform.m_scale.x;
			Vector3 rowY = Vector3::AxisY; m_groundTiltRotation.Apply(rowY); rowY = rowY * m_transform.m_scale.y;
			Vector3 rowZ = Vector3::AxisZ; m_groundTiltRotation.Apply(rowZ); rowZ = rowZ * m_transform.m_scale.z;

			Matrix tiltedWorld;
			tiltedWorld.v[0].Set(rowX.x, rowX.y, rowX.z, 0.0f);
			tiltedWorld.v[1].Set(rowY.x, rowY.y, rowY.z, 0.0f);
			tiltedWorld.v[2].Set(rowZ.x, rowZ.y, rowZ.z, 0.0f);
			tiltedWorld.v[3].Set(m_transform.m_position.x, m_transform.m_position.y, m_transform.m_position.z, 1.0f);

			m_skeleton.Update(tiltedWorld);
		}


		void Enemy::InitLegIK()
		{
			bool ok = true;

			ok &= m_legIK.AddLegByBoneNames(
				&m_skeleton, L"Hip_L", L"Knee_L", L"Ankle_L",
				Vector3(0.0f, -1.0f, 1.0f), 1.0f, 0.0f,
				{ L"Toes1_L", L"MiddleToe1_L", L"MiddleToe2_L" });

			ok &= m_legIK.AddLegByBoneNames(
				&m_skeleton, L"Hip_R", L"Knee_R", L"Ankle_R",
				Vector3(0.0f, -1.0f, 1.0f), 1.0f, 0.0f,
				{ L"Toes1_R", L"MiddleToe1_R", L"MiddleToe2_R" });

			ok &= m_legIK.AddLegByBoneNames(
				&m_skeleton, L"Shoulder_L", L"Elbow_L", L"Wrist_L",
				Vector3(0.0f, -1.0f, -1.0f), 1.0f, 0.0f,
				{ L"Fingers1_L", L"MiddleFinger1_L", L"MiddleFinger2_L" },
				{ L"ShoulderPart1_L" },
				{ L"ElbowPart1_L", L"ElbowPart2_L" });

			ok &= m_legIK.AddLegByBoneNames(
				&m_skeleton, L"Shoulder_R", L"Elbow_R", L"Wrist_R",
				Vector3(0.0f, -1.0f, -1.0f), 1.0f, 0.0f,
				{ L"Fingers1_R", L"MiddleFinger1_R", L"MiddleFinger2_R" },
				{ L"ShoulderPart1_R" },
				{ L"ElbowPart1_R", L"ElbowPart2_R" });

			// ボーン名のtypoやリグ変更にすぐ気づけるよう、開発中は止める
			// （NDEBUGビルド＝リリース版ではassertは無効化されるので本番影響なし）
			assert(ok && "InitLegIK: ボーンが見つからず脚IKの登録に失敗した箇所があります");
		}


		bool Enemy::ShouldSuppressFootprint() const
		{
			return m_stateMachine->IsSwim();
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
