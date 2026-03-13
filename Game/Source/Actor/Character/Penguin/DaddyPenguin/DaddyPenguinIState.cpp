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


namespace app
{
	namespace actor
	{

		DaddyPenguinIState::DaddyPenguinIState(DaddyPenguinStateMachine* owner)
			: m_owner(owner)
		{
		}




		/************************************/


		void DaddyPenguinIdleState::Enter()
		{
		}


		void DaddyPenguinIdleState::Update()
		{
		}


		void DaddyPenguinIdleState::Exit()
		{
		}


		DaddyPenguinIdleState::DaddyPenguinIdleState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{
		}




		/************************************/


		void DaddyPenguinMoveState::Enter()
		{
		}


		void DaddyPenguinMoveState::Update()
		{
		}


		void DaddyPenguinMoveState::Exit()
		{
		}


		DaddyPenguinMoveState::DaddyPenguinMoveState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{
		}
	}
}