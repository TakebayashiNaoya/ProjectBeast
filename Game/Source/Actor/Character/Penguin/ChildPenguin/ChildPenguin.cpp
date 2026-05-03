/**
 * @file ChildPenguin.cpp
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinManager.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Core/ParameterManager.h"
#include "Physics/Physics.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/childPenguin/ChildPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};

			/** 地形法線に沿わせる補間速度 */
			constexpr float SLIDE_TILT_SLERP_SPEED = 10.0f;
			/** 真下レイの射程距離 */
			constexpr float SLIDE_RAY_LENGTH = 200.0f;
		}


		float ChildPenguin::GetJoinDistance() const
		{
			if (m_aiController)
			{
				return m_aiController->GetJoinDistance();
			}
			return 0.0f;
		}


		void ChildPenguin::SetChildPenguinType(EnChildPenguinType type)
		{
			m_type = type;
			m_colorApplied = false;

			/** タイプ別乗算カラーをJSONパラメーターから設定 */
			const int typeIndex = static_cast<int>(type);
			const auto* param = core::ParameterManager::Get()->GetParameter<MasterChildPenguinParameter>(typeIndex);
			const auto& td = param->typeData[typeIndex];
			m_typeColor = Vector4(td.colorR, td.colorG, td.colorB, td.colorA);

			/** タイプ変更に伴いステートマシンを作成 */
			/** おっちょこちょいタイプは固有ステートを持つため専用クラスを使う */
			if (m_type == EnChildPenguinType::Clumsy)
			{
				m_stateMachine = std::make_unique<ClumsyChildPenguinStateMachine>(this);
			}
			else
			{
				m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			}
			m_characterStateMachine = m_stateMachine.get();

			CreateAIController();
		}


		void ChildPenguin::CreateAIController()
		{
			/** 親ペンギンが設定されたら、タイプに応じたAIコントローラーを作成 */
			switch (m_type)
			{
			case EnChildPenguinType::Serious:
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clingy:
				m_aiController = std::make_unique<ClingyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Naughty:
				m_aiController = std::make_unique<NaughtyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clumsy:
				m_aiController = std::make_unique<ClumsyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Caring:
				m_aiController = std::make_unique<CaringChildPenguinAI>(this);
				break;
			}
		}


		ChildPenguin::ChildPenguin()
		{
			Init(MODEL_DATA);

			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			m_status = std::make_unique<ChildPenguinStatus>();
			m_status->Setup();

			CreateAIController();
		}


		void ChildPenguin::Start()
		{
			/** スケールを初期化 */
			m_transform.m_scale = CHILD_PENGUIN_SCALE;
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			/** モデルロード完了後、一度だけカラーを適用 */
			if (m_modelReady && !m_colorApplied)
			{
				m_modelRender.SetMulColor(m_typeColor);
				m_modelRender.Update();
				m_colorApplied = true;
			}

			/** AIコントローラーがあれば更新 */
			if (m_aiController)
			{
				m_aiController->Update();
			}

			/** ステートマシン更新 */
			m_stateMachine->Update();

			PenguinBase::Update();

			/** 泳ぎ中はモデルの描画位置のみY座標をオフセットする */
			/** 物理・ステート判定には影響を与えない */
			if (m_stateMachine->IsSwimming() && m_modelReady)
			{
				Vector3 renderPos = m_transform.m_position;
				renderPos.y += SWIM_Y_OFFSET;
				m_modelRender.SetTRS(renderPos, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
				return;
			}

			/** スライド中は地形の法線に沿ってモデルを傾ける */
			/** 物理・ステート判定には影響を与えない */
			if (m_stateMachine->IsEqualCurrentState(PenguinSlidingState::ID()) && m_modelReady)
			{
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
				return;
			}

			/** スライド以外のステートでは補間用の回転をリセットする */
			m_slideModelRotation = m_transform.m_rotation;
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}