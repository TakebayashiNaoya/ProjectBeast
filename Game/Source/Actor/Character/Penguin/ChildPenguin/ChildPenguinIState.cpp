/**
 * @file ChildPenguinIState.cpp
 * @brief 子ペンギンのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinIState.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"


namespace app
{
	namespace actor
	{

		ChildPenguinIState::ChildPenguinIState(ChildPenguinStateMachine* owner)
			: m_owner(owner)
		{}




		/************************************/


		void ChildPenguinIdleState::Enter()
		{}


		void ChildPenguinIdleState::Update()
		{}


		void ChildPenguinIdleState::Exit()
		{}


		ChildPenguinIdleState::ChildPenguinIdleState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}




		/************************************/


		void ChildPenguinMoveState::Enter()
		{}


		void ChildPenguinMoveState::Update()
		{}


		void ChildPenguinMoveState::Exit()
		{}


		ChildPenguinMoveState::ChildPenguinMoveState(ChildPenguinStateMachine* owner)
			: ChildPenguinIState(owner)
		{}
	}
}