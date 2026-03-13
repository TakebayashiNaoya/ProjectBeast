/**
 * @file EnemyController.h
 * @brief エネミーのコントローラー
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Enemy;
		class Player;

		/**
		 * @brief エネミーのコントローラークラス
		 */
		class EnemyController :public Noncopyable
		{
		public:
			EnemyController();
			~EnemyController();

			bool Start();
			void Update();
			void Render(RenderContext& renderContext);


		public:
			/** 操作対象の設定 */
			void SetTarget(Enemy* target)
			{
				m_target = target;
			}


		public:
			static void Initialize();


		private:
			/**
			 * 関数ポインタ
			 */
			using EnterFunc = void (*)(EnemyController*);
			using UpdateFunc = void (*)(EnemyController*);
			using ExitFunc = void (*)(EnemyController*);
			using CheckFunc = int (*)(EnemyController*);

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
			 * @enum EnEnemyState
			 */
			enum EnEnemyStateID
			{
				enEnemyState_Idle,
				enEnemyState_Move,
				enEnemyState_Jump,
				enEnemyState_Swim,
				enEnemyState_Attack,

				enEnemyState_Num,
				enEnemyState_Invalid = -1
			};


		private:
			void ChangeState(EnEnemyStateID nextState);


		private:
			static std::map<EnEnemyStateID, AIState> m_stateMap;


		private:
			/**
			 * @brief AIStateを登録する関数
			 * @param id ステートのID
			 * @param enter Enter関数
			 * @param update Update関数
			 * @param exit Exit関数
			 * @param check Check関数
			 */
			static void RegisterState(const EnEnemyStateID id, EnterFunc enter, UpdateFunc update, ExitFunc exit, CheckFunc check)
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
			AIState* FindAIState(const EnEnemyStateID id)
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
			static void  DoNothing(EnemyController*) {};
			/** 遷移なし */
			static int CheckNothing(EnemyController*) { return -1; }


		private:
			/**
			 * 待機
			 */
			static void EnterIdle(EnemyController* enemy);
			static void UpdateIdle(EnemyController* enemy);
			static void ExitIdle(EnemyController* enemy);
			static int CheckIdle(EnemyController* enemy);


			/**
			 * 移動
			 */
			static void EnterMove(EnemyController* enemy);
			static void UpdateMove(EnemyController* enemy);
			static void ExitMove(EnemyController* enemy);
			static int CheckMove(EnemyController* enemy);


			/**
			 * ジャンプ
			 */
			static void EnterJump(EnemyController* enemy);
			static void UpdateJump(EnemyController* enemy);
			static void ExitJump(EnemyController* enemy);
			static int CheckJump(EnemyController* enemy);


			/**
			 * 泳ぐ
			 */
			static void EnterSwim(EnemyController* enemy);
			static void UpdateSwim(EnemyController* enemy);
			static void ExitSwim(EnemyController* enemy);
			static int CheckSwim(EnemyController* enemy);


			/**
			 * 攻撃
			 */
			static void EnterAttack(EnemyController* enemy);
			static void UpdateAttack(EnemyController* enemy);
			static void ExitAttack(EnemyController* enemy);
			static int CheckAttack(EnemyController* enemy);


		private:
			Enemy* m_target;

			/** targetの前回の位置を保持 */
			Vector3 prePosition = Vector3::Zero;

			Vector3 m_targetPosition = Vector3::Zero;
			bool isFind = false;
			/** 現在の状態 */
			EnEnemyStateID m_currentState = enEnemyState_Idle;
			/** 初期化処理をしたか */
			bool m_isInitialized = false;
		};
	}
}

