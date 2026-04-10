/**
 * @file Whirlpool.cpp
 * @brief 渦潮クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "types.h"
#include "Whirlpool.h"


namespace app
{
	namespace actor
	{
		void Whirlpool::Start()
		{
			Init("Assets/modelData/stage/Whirlpool/whirlpool.tkm");

			m_scaleBigger.Initialize(MIN_SCALE, MAX_SCALE, SCALE_CHANGE_TIME, util::EasingType::EaseInOut, util::LoopMode::Once);
			m_scaleSmaller.Initialize(MAX_SCALE, MIN_SCALE, SCALE_CHANGE_TIME, util::EasingType::EaseInOut, util::LoopMode::Once);
		}


		void Whirlpool::Update()
		{
			StateMachine();
		}


		void Whirlpool::Render(RenderContext& rc)
		{
			IStageObject::Render(rc);
		}

		Whirlpool::Whirlpool()
			: m_state(EnWhirlpoolState::ModelLoading)
			, m_timer(0.0f)
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