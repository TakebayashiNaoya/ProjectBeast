/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシンの基底クラス群
 * @author 藤谷
 */
#pragma once
#include "Source/Util/CRC32.h"
 /**
  * @brief 数値に変換するマクロ
  * @param name ステート名
  */
#define appState(name)\
public:\
 static constexpr uint32_t ID() { return Hash32(#name); }


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class StateMachineBase;


		/**
		 * @brief ステートの基底クラス
		 */
		class IState
		{
		public:
			IState() = default;
			virtual ~IState() = default;


		public:
			/** ステートに入ったときの処理 */
			virtual void Enter() = 0;
			/** ステートの更新処理 */
			virtual void Update() = 0;
			/** ステートから出るときの処理 */
			virtual void Exit() = 0;
		};




		/*************************************************************/


		/**
		 * @brief アクターのステートマシンの基底クラス
		 */
		class StateMachineBase
		{
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
			template<typename TState, typename TStateMachine>
			void AddState(TStateMachine&& stateMachine)
			{
				// IDを取得
				const uint32_t stateID = TState::ID();
				// 指定したIDを取得
				auto it = m_stateMap.find(stateID);
				// IDが外れ値の場合
				if (it != m_stateMap.end())
				{
					// 既存のものを削除して警告を出す
					delete it->second.get();
					K2_ASSERT(false, "重複しています");
				}
				// ステートを追加
				m_stateMap[stateID] = std::make_unique<TState>(std::forward<TStateMachine>(stateMachine));
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
			IState* FindState(const uint32_t stateID);


		public:
			StateMachineBase();
			virtual ~StateMachineBase() = default;


		protected:
			/** ステートマップ */
			std::unordered_map<uint32_t, std::unique_ptr<IState>> m_stateMap;
			/** 現在のステート */
			IState* m_currentState;
			/** 次のステート */
			IState* m_nextState;
		};
	}
}

