/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシンの基底クラス群
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class ActorStateMachine;


		/**
		 * @brief ステートの基底クラス
		 */
		class IState
		{
		private:
			/** ステートマシン */
			ActorStateMachine* m_stateMachine;


		public:
			IState();
			virtual ~IState() = default;


		public:
			/** ステートに入ったときの処理 */
			virtual void Enter() = 0;
			/** ステートの更新処理 */
			virtual void Update() = 0;
			/** ステートから出るときの処理 */
			virtual void Exit() = 0;


		protected:
			/**
			 * @brief ステートマシンを取得する
			 * @tparam TStateMachine ステートマシンの型
			 */
			template<typename TStateMachine>
			TStateMachine* GetOwner()
			{
				return dynamic_cast<TStateMachine*>(m_stateMachine);
			}
		};




		/*************************************************************/


		/**
		 * @brief アクターのステートマシンの基底クラス
		 */
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
			ActorStateMachine();
			virtual ~ActorStateMachine() = default;


		public:
			/*
			 * @brief ステートマシンを更新する
			 * @note 持ち主が毎フレーム呼び出すこと
			 */
			void Update();


			/** ステートを変更させる */
			void ChangeState();


		protected:
			/**
			 * @brief ステートを追加する
			 * @tparam TState ステートの型
			 * @param stateID ステートID
			 */
			template<typename TState>
			void AddState(const uint8_t stateID)
			{
				// 指定したIDを取得
				auto it = m_stateMap.find(stateID);
				// IDが外れ値の場合
				if (it == m_stateMap.end())
				{
					// 既存のものを削除して警告を出す
					delete it->second;
					K2_ASSERT(false, "重複しています");
				}
				// ステートを追加
				m_stateMap[stateID] = std::make_unique<TState>(this);
			}


			/*
			 * @brief ステートの変更先を取得する
			 * @return 変更先のステートポインタ
			 */
			virtual IState* GetChangeState() = 0;


			/**
			 * @brief ステートを取得する
			 * @param stateID ステートID
			 * @return ステートポインタ
			 */
			IState* FindState(const uint8_t stateID);
		};
	}
}

