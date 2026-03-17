/**
 * @file ActorStateMachine.cpp
 * @brief アクターのステートマシン
 * @author 藤谷
 */
#include "stdafx.h"
#include "ActorStateMachine.h"


namespace app
{
	namespace actor
	{
		core::IState* ActorStateMachine::GetChangeState()
		{
			// 何もしない
			return nullptr;
		}


		ActorStateMachine::ActorStateMachine(Actor* ownerActor)
			: m_ownerActor(ownerActor)
		{}
	}
}