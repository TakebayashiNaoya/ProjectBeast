/**
 * @file NPCController.cpp
 * @brief NPCのコントローラー
 * @author 立山
 */
#include "stdafx.h"
#include "NPCController.h"


namespace app
{
	// Static変数の初期化
	std::map<NPCController::EnAIStateID, NPCController::AIState> NPCController::m_stateMap;


	NPCController::NPCController()
	{
	}


	NPCController::~NPCController()
	{
	}


	void NPCController::Start()
	{
		StartWrapper();
	}


	void NPCController::Update()
	{
		UpdateWrapper();
		auto* currentState = FindAIState(m_currentState);
		if (currentState == nullptr) {
			K2_ASSERT(false, "対象の処理が見つかりません\n");
			return;
		}

		/** 初回起動時のEnter処理 */
		if (!m_isInitialized) {
			currentState->enter(this);
			m_isInitialized = true;
		}

		/** 遷移判定 */
		const int nextState = currentState->check(this);
		if (nextState != -1 && nextState != m_currentState) {
			ChangeState((EnAIStateID)nextState);
			currentState = FindAIState(m_currentState);
		}

		/** 現在のステートのアップデート */
		currentState->update(this);

	}


	void NPCController::Render(RenderContext& renderContext)
	{
		//RenderWrapper(renderContext);
	}


	void NPCController::ChangeState(EnAIStateID nextState)
	{
		/** 指定したnextStateがおかしい */
		if (nextState < enAIState_Invalid || nextState >= enAIState_Num) {
			return;
		}

		auto* currentState = FindAIState(m_currentState);
		/** 現在のステートのExit処理 */
		currentState->exit(this);
		/** ステートの更新 */
		m_currentState = nextState;
		/** 新しいステートのEnterを呼ぶ */
		currentState = FindAIState(m_currentState);
		currentState->enter(this);
	}


	/** 各ステートの実装 */
	void NPCController::Initialize()
	{
		/** 待機 */
		RegisterState(enAIState_Idle, EnterIdle, UpdateIdle, ExitIdle, CheckIdle);
		/** 移動 */
		RegisterState(enAIState_Move, EnterMove, UpdateMove, ExitMove, CheckMove);
		/** ジャンプ */
		RegisterState(enAIState_Jump, EnterJump, UpdateJump, ExitJump, CheckJump);
		/** 泳ぐ */
		RegisterState(enAIState_Swim, EnterSwim, UpdateSwim, ExitSwim, CheckSwim);
	}


	/** 各ステートの処理はこの下に書いていく */

	/** 待機 */
	void NPCController::EnterIdle(NPCController* npc)
	{

	}

	void NPCController::UpdateIdle(NPCController* npc)
	{
	}

	void NPCController::ExitIdle(NPCController* npc)
	{
	}

	int NPCController::CheckIdle(NPCController* npc)
	{
		return enAIState_Invalid;
	}


	/** 移動 */
	void NPCController::EnterMove(NPCController* npc)
	{
	}

	void NPCController::UpdateMove(NPCController* npc)
	{
	}

	void NPCController::ExitMove(NPCController* npc)
	{
	}

	int NPCController::CheckMove(NPCController* npc)
	{
		return enAIState_Invalid;
	}


	/** ジャンプ */
	void NPCController::EnterJump(NPCController* npc)
	{
	}

	void NPCController::UpdateJump(NPCController* npc)
	{
	}

	void NPCController::ExitJump(NPCController* npc)
	{
	}

	int NPCController::CheckJump(NPCController* npc)
	{
		return enAIState_Invalid;
	}


	/** 泳ぐ */
	void NPCController::EnterSwim(NPCController* npc)
	{
	}

	void NPCController::UpdateSwim(NPCController* npc)
	{
	}

	void NPCController::ExitSwim(NPCController* npc)
	{
	}

	int NPCController::CheckSwim(NPCController* npc)
	{
		return enAIState_Invalid;
	}
}