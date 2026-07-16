/**
 * @file PenguinBase.cpp
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"

#include "PenguinBase.h"
#include "Physics/Physics.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinEffectStatus.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Effect/DecalManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 地形法線に沿わせる補間速度 */
			constexpr float SLIDE_TILT_SLERP_SPEED = 10.0f;
			/** 真下レイの射程距離 */
			constexpr float SLIDE_RAY_LENGTH = 200.0f;
		}


		PenguinBase::PenguinBase()
		{
			m_effectStatus = std::make_unique<PenguinEffectStatus>();
			m_effectStatus->Setup();
		}


		void PenguinBase::Start()
		{
			CharacterBase::Start();
		}


		void PenguinBase::Update()
		{
			CharacterBase::Update();
			UpdateFootprints();
		}

		void PenguinBase::UpdateFootprints()
		{
			// ジャンプ中、泳ぎ中、スライド中は出さない
			if (m_characterStateMachine->IsEqualCurrentState(PenguinJumpState::ID()) ||
				m_characterStateMachine->IsEqualCurrentState(PenguinSwimmingState::ID()) ||
				m_characterStateMachine->IsEqualCurrentState(PenguinSlidingState::ID()))
			{
				m_lastFootprintPos = m_transform.m_position;
				return;
			}

			Vector3 currentPos = m_transform.m_position;

			// ★演算子 '-' の代わりに Subtract メソッドを使用
			Vector3 diff;
			diff.Subtract(currentPos, m_lastFootprintPos);
			diff.y = 0.0f;

			// 距離の二乗比較
			if (diff.LengthSq() > 15.0f * 15.0f)
			{
				// 向きの計算
				Vector3 forward = Vector3::AxisZ;
				m_transform.m_rotation.Apply(forward);
				float yaw = atan2f(forward.x, forward.z);

				// 左右のズレ（右方向ベクトルの取得）
				Vector3 right = Vector3::AxisX;
				m_transform.m_rotation.Apply(right);

				// ★演算子 '*' の代わりに Scale メソッドを使用
				float offsetAmount = 3.0f;
				if (!m_isRightFoot) offsetAmount *= -1.0f; // 左足なら反転
				right.Scale(offsetAmount);

				// ★演算子 '+' の代わりに Add メソッドを使用
				Vector3 spawnPos;
				spawnPos.Add(currentPos, right);

				float size = GetFootprintSize();

				app::effect::DecalManager::Get().SpawnFootprint(
					spawnPos, yaw, app::effect::DecalKind::SnowFootprint,
					size, 1.0f, 0.5f, { 0.2f, 0.5f, 1.0f, 1.0f }
				);

				m_lastFootprintPos = currentPos;
				m_isRightFoot = !m_isRightFoot;
			}
		}

		void PenguinBase::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}


		void PenguinBase::UpdateSlideTilt()
		{
			/** スライドステート中かつモデルロード完了済みのときのみ処理する */
			if (!m_characterStateMachine->IsEqualCurrentState(PenguinSlidingState::ID()) || !m_modelReady)
			{
				/** スライド以外のステートでは補間用の回転をリセットする */
				m_slideModelRotation = m_transform.m_rotation;
				return;
			}

			/** 真下にレイを飛ばして地形法線を取得する */
			const Vector3& pos = m_transform.m_position;
			const Vector3 rayStart = pos;
			const Vector3 rayEnd = Vector3(pos.x, pos.y - SLIDE_RAY_LENGTH, pos.z);

			nsBeastEngine::nsCollision::RaycastHit hit;
			Vector3 groundNormal = Vector3::Up;
			if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit))
			{
				groundNormal = hit.normal;
			}

			/** ステップ1：進行方向からY軸回転（水平の向き）を求める */
			Quaternion yRot = m_transform.m_rotation;
			Vector3 velocity = m_characterStateMachine->GetCurrentVelocity();
			velocity.y = 0.0f;
			if (velocity.LengthSq() > FLT_EPSILON)
			{
				yRot.SetRotationYFromDirectionXZ(velocity);
			}

			/** ステップ2：Up → groundNormal への最短回転を求める */
			/** この回転がそのまま地形の傾きを表す */
			/** SetRotation(from, to) はエンジン既存関数 */
			Quaternion tiltRot;
			tiltRot.SetRotation(Vector3::Up, groundNormal);

			/** ステップ3：傾き回転（ワールド空間）をY軸回転に前から合成する */
			/** tiltRot * yRot の順で乗算することで、 */
			/** 「まず地形に合わせて傾け、次に進行方向を向く」合成になる */
			Quaternion targetRotation;
			targetRotation.Multiply(yRot, tiltRot);

			/** 補間して急激な回転変化を抑える */
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const float slerpFactor = min(1.0f, SLIDE_TILT_SLERP_SPEED * deltaTime);
			m_slideModelRotation.Slerp(slerpFactor, m_slideModelRotation, targetRotation);

			m_modelRender.SetTRS(m_transform.m_position, m_slideModelRotation, m_transform.m_scale);
			m_modelRender.Update();
		}
	}
}