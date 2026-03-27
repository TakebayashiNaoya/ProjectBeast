/**
 * @file DaddyPenguinIState.cpp
 * @brief 親ペンギンのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/LoseCamera.h"
#include "Source/Camera/WinCamera.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{

		DaddyPenguinIState::DaddyPenguinIState(DaddyPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void DaddyPenguinCommandShoutState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);
			EffectManager::Get().PlayEffect(EnEffectKind::DaddyPenguinCommand, m_owner->GetTransform().m_position, Quaternion::Identity, Vector3::One);
		}


		void DaddyPenguinCommandShoutState::Update()
		{}


		void DaddyPenguinCommandShoutState::Exit()
		{}


		DaddyPenguinCommandShoutState::DaddyPenguinCommandShoutState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinWinState::Enter()
		{
			float ry = m_owner->GetTransform().m_rotation.y;

			Vector3 front;
			front.x = sinf(ry);
			front.y = 0.0f;
			front.z = cosf(ry);


			// ターゲットを親ペンギンの座標に設定
			camera::CameraManager::Get().GetController<camera::WinCamera>(camera::WinCamera::ID())->SetTarget(m_owner->GetTransform().m_position, front);

			// 勝利カメラに切り替え
			camera::CameraManager::Get().SwitchCamera(camera::WinCamera::ID(), 1.0f);
		}


		void DaddyPenguinWinState::Update()
		{}


		void DaddyPenguinWinState::Exit()
		{}


		DaddyPenguinWinState::DaddyPenguinWinState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinLoseState::Enter()
		{
			// ターゲットを親ペンギンの座標に設定
			camera::CameraManager::Get().GetController<camera::LoseCamera>(camera::LoseCamera::ID())->SetTarget(m_owner->GetTransform().m_position);

			// 負けカメラに切り替え
			camera::CameraManager::Get().SwitchCamera(camera::LoseCamera::ID(), 1.0f);
		}


		void DaddyPenguinLoseState::Update()
		{
			m_timer += g_gameTime->GetFrameDeltaTime();

			if (m_timer >= 2.5f)
			{
				// ターゲットを親ペンギンの座標に設定
				camera::CameraManager::Get().GetController<camera::DefeatCamera>(camera::DefeatCamera::ID())->SetTarget(m_owner->GetTransform().m_position);
				// 負けカメラに切り替え
				camera::CameraManager::Get().SwitchCamera(camera::DefeatCamera::ID(), 1.0f);
				m_timer = 0.0f;
			}
		}


		void DaddyPenguinLoseState::Exit()
		{}


		DaddyPenguinLoseState::DaddyPenguinLoseState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}
	}
}