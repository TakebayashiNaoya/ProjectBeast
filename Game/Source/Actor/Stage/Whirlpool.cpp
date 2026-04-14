/**
 * @file Whirlpool.cpp
 * @brief 渦潮クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Whirlpool.h"

namespace app
{
	namespace actor
	{

		namespace
		{
			/** 渦潮の回転速度 */
			constexpr float ROTATION_SPEED = -3.0f;
			/** 渦潮の拡大率の変化にかかる時間 */
			constexpr float SCALE_CHANGE_TIME = 2.5f;
			/** 渦潮の拡大率が最大値で留まる時間 */
			constexpr float WHIRLPOOL_STAY_TIME = 10.0f;
			/** 渦潮の最小値 */
			const Vector3 MIN_SCALE = Vector3(0.0f, 0.0f, 0.0f);
			/** 渦潮の最大値 */
			const Vector3 MAX_SCALE = Vector3(5.0f, 5.0f, 5.0f);
			/** フェードアウトにかかる時間 */
			constexpr float FADE_OUT_TIME = 3.0f;
		}

		void Whirlpool::Start()
		{
			m_whirlpoolPowerSystem = std::make_unique<WhirlpoolPowerSytem>(this);

			Init("Assets/modelData/stage/Whirlpool/whirlpool.tkm");

			m_scaleBigger.Initialize(MIN_SCALE, MAX_SCALE, SCALE_CHANGE_TIME, util::EasingType::EaseInOut, util::LoopMode::Once);
			m_scaleSmaller.Initialize(MAX_SCALE, MIN_SCALE, SCALE_CHANGE_TIME, util::EasingType::EaseInOut, util::LoopMode::Once);

			m_whirlpoolPowerSystem->Start();
		}


		void Whirlpool::Update()
		{
			StateMachine();
			m_whirlpoolPowerSystem->Update();
		}


		void Whirlpool::Render(RenderContext& rc)
		{
			IStageObject::Render(rc);
			m_whirlpoolPowerSystem->Render(rc);
		}

		Whirlpool::Whirlpool()
			: m_state(EnWhirlpoolState::ModelLoading)
			, m_timer(0.0f)
			, m_index(0)
		{}


		void Whirlpool::StateMachine()
		{
			float deltaTime = g_gameTime->GetFrameDeltaTime();

			switch (m_state)
			{
			case EnWhirlpoolState::ModelLoading:
			{
				if (IsLoaded())
				{
					m_state = EnWhirlpoolState::Bigger;
					m_scaleBigger.Play();
				}
				break;
			}
			case EnWhirlpoolState::Bigger:
			{
				m_scaleBigger.Update(deltaTime);
				m_transform.m_rotation.AddRotationDegY(ROTATION_SPEED);
				m_transform.m_scale = m_scaleBigger.GetCurrentValue();

				if (!m_scaleBigger.IsPlaying())
				{
					m_state = EnWhirlpoolState::Stay;
				}

				break;
			}
			case EnWhirlpoolState::Stay:
			{
				m_timer += deltaTime;
				m_transform.m_rotation.AddRotationDegY(ROTATION_SPEED);

				if (m_timer >= WHIRLPOOL_STAY_TIME)
				{
					m_timer = 0.0f;
					m_state = EnWhirlpoolState::Smaller;
					m_scaleSmaller.Play();
				}

				break;
			}
			case EnWhirlpoolState::Smaller:
			{
				m_scaleSmaller.Update(deltaTime);
				m_transform.m_rotation.AddRotationDegY(ROTATION_SPEED);
				m_transform.m_scale = m_scaleSmaller.GetCurrentValue();

				if (!m_scaleSmaller.IsPlaying())
				{
					m_state = EnWhirlpoolState::None;
				}

				break;
			}
			case EnWhirlpoolState::None:
			{



			}
			break;
			default:
				break;
			}

			// 以下共通処理
			IStageObject::Update();
		}
	}
}