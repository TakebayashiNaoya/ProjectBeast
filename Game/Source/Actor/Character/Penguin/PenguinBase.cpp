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