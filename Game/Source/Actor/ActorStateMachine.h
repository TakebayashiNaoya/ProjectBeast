#pragma once




namespace app
{
	namespace actor
	{

		// 前方宣言
		class ActorStateMachine;


		class IState
		{
		private:
			/** ステートマシン */
			ActorStateMachine* m_stateMachine;


		public:
			IState();
			virtual ~IState() = default;


		public:
			virtual void Enter() = 0;
			virtual void Update() = 0;
			virtual void Exit() = 0;


		protected:
			/** ステートマシンを取得する */
			template<typename TStateMachine>
			TStateMachine* GetOwner()
			{
				return dynamic_cast<TStateMachine*>(m_stateMachine);
			}
		};


		class ActorStateMachine
		{
			// ステートのマップ
			// キー: uint8_t
			// 中身: std::unique_ptr<IState>
			using StateMap = std::unordered_map<uint8_t, std::unique_ptr<IState>>;
		private:
			/** ステートマップ */
			StateMap m_stateMap;
			/** 現在のステート */
			IState* m_currentState;
			/** 次のステート */
			IState* m_nextState;


		public:
			/*
			 * ステートマシンを更新する
			 * NOTE: ステートマシンの持ち主が毎フレーム呼び出すこと
			 */
			void Update();


			/** ステートを変更させる */
			void ChangeState();


		protected:
			/*
			 * ステートの変更先を取得する
			 */
			virtual IState* GetChangeState() = 0;


			/** ステートを取得する */
			IState* FindState(const uint8_t stateID);


			/** ステートを追加する */
			template<typename TState>
			void AddState(const uint8_t stateID)
			{
				auto it = m_stateMap.find(stateID);
				if (it == m_stateMap.end())
				{
					delete it->second;
					K2_ASSERT(false, "重複しています");
				}
				m_stateMap[stateID] = std::make_unique<TState>(this);
			}


		public:
			ActorStateMachine();
			virtual ~ActorStateMachine() = default;
		};
	}
}

