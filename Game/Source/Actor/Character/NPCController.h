/**
 * @file NPCController.h
 * @brief NPCのコントローラー
 * @author 立山
 */
#pragma once
#include "IObject.h"


namespace app {

	class NPCController :public IObject
	{

	public:
		NPCController();
		~NPCController();
		void Start() override;
		void Update() override;
		void Render(RenderContext& renderContext) override;


	public:
		static void Initialize();


	private:
		/**
		 * 関数ポインタ
		 */
		using EnterFunc = void (*)(NPCController*);
		using UpdateFunc = void (*)(NPCController*);
		using ExitFunc = void (*)(NPCController*);
		using CheckFunc = int (*)(NPCController*);

	private:
		/** AI思考処理 */
		struct AIState
		{
			EnterFunc enter;
			UpdateFunc update;
			ExitFunc exit;
			CheckFunc check;
		};


	private:
		/**
		 * @enum EnAIStateID
		 * 思考パターンのID
		*/
		enum EnAIStateID
		{
			enAIState_Idle,
			enAIState_Move,
			enAIState_Jump,
			enAIState_Swim,

			enAIState_Num,
			enAIState_Invalid = -1
		};


	private:
		void ChangeState(EnAIStateID nextState);


	private:
		static std::map<EnAIStateID, AIState> m_stateMap;


	private:
		/**
		 * @brief AIStateを登録する関数
		 * @param id ステートのID
		 * @param enter Enter関数
		 * @param update Update関数
		 * @param exit Exit関数
		 * @param check Check関数
		 */
		static void RegisterState(const EnAIStateID id, EnterFunc enter, UpdateFunc update, ExitFunc exit, CheckFunc check)
		{
			AIState state;
			/** 引数が nullptr ならDoNothingを入れる */
			state.enter = (enter != nullptr) ? enter : DoNothing;
			state.update = (update != nullptr) ? update : DoNothing;
			state.exit = (exit != nullptr) ? exit : DoNothing;
			state.check = (check != nullptr) ? check : CheckNothing;
			// mapに登録
			m_stateMap.emplace(id, state);
		}


		/** AIStateを探す */
		AIState* FindAIState(const EnAIStateID id)
		{
			auto it = m_stateMap.find(id);
			if (it != m_stateMap.end()) {
				return &it->second;
			}
			return nullptr;
		}


		/**
		 * 何もしない関数
		 */
		static void  DoNothing(NPCController*) {};
		/** 遷移なし */
		static int CheckNothing(NPCController*) { return -1; }


	private:
		/**
		 * 待機
		 */
		static void EnterIdle(NPCController* npc);
		static void UpdateIdle(NPCController* npc);
		static void ExitIdle(NPCController* npc);
		static int CheckIdle(NPCController* npc);


		/**
		 * 移動
		 */
		static void EnterMove(NPCController* npc);
		static void UpdateMove(NPCController* npc);
		static void ExitMove(NPCController* npc);
		static int CheckMove(NPCController* npc);


		/**
		 * ジャンプ
		 */
		static void EnterJump(NPCController* npc);
		static void UpdateJump(NPCController* npc);
		static void ExitJump(NPCController* npc);
		static int CheckJump(NPCController* npc);


		/**
		 * 泳ぐ
		 */
		static void EnterSwim(NPCController* npc);
		static void UpdateSwim(NPCController* npc);
		static void ExitSwim(NPCController* npc);
		static int CheckSwim(NPCController* npc);


	private:

		/** 現在の状態 */
		EnAIStateID m_currentState = enAIState_Idle;
		/** 初期化処理をしたか */
		bool m_isInitialized = false;

	};
}