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
		class ChildPenguin;

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
			void SetTarget(Enemy* target);

		public:
			void AddTargetPos(const Vector3& pos);

		public:
			static void Initialize();

		public:
			void SetStun(const bool isStun) { m_isStun = isStun; }

		private:
			/** 子ペンギンを探す */
			ChildPenguin* FindTarget();

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
				enEnemyState_Stun,
				enEnemyState_Search,
				enEnemyState_Wandering,
				enEnemyState_Chase,
				enEnemyState_Jump,
				enEnemyState_Swim,
				enEnemyState_Attack,
				enEnemyState_ReturnHome,
				enEnemyState_CoolDown,

				enEnemyState_Num,
				enEnemyState_Invalid = -1
			};

		private:
			void ChangeState(EnEnemyStateID nextState);

		private:
			static std::map<EnEnemyStateID, AIState> m_stateMap;

		private:
			bool IsFarFromHome()const;

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
			/** 待機 */
			static void EnterIdle(EnemyController* enemy);
			static void UpdateIdle(EnemyController* enemy);
			static void ExitIdle(EnemyController* enemy);
			static int CheckIdle(EnemyController* enemy);

			/** スタン */
			static void EnterStun(EnemyController* enemy);
			static void UpdateStun(EnemyController* enemy);
			static void ExitStun(EnemyController* enemy);
			static int CheckStun(EnemyController* enemy);

			/** サーチ */
			static void EnterSearch(EnemyController* enemy);
			static void UpdateSearch(EnemyController* enemy);
			static void ExitSearch(EnemyController* enemy);
			static int CheckSearch(EnemyController* enemy);

			/** 徘徊 */
			static void EnterWandering(EnemyController* enemy);
			static void UpdateWandering(EnemyController* enemy);
			static void ExitWandering(EnemyController* enemy);
			static int CheckWandering(EnemyController* enemy);

			/** チェイス */
			static void EnterChase(EnemyController* enemy);
			static void UpdateChase(EnemyController* enemy);
			static void ExitChase(EnemyController* enemy);
			static int CheckChase(EnemyController* enemy);

			/** ジャンプ */
			static void EnterJump(EnemyController* enemy);
			static void UpdateJump(EnemyController* enemy);
			static void ExitJump(EnemyController* enemy);
			static int CheckJump(EnemyController* enemy);

			/** 泳ぐ */
			static void EnterSwim(EnemyController* enemy);
			static void UpdateSwim(EnemyController* enemy);
			static void ExitSwim(EnemyController* enemy);
			static int CheckSwim(EnemyController* enemy);

			/** 攻撃 */
			static void EnterAttack(EnemyController* enemy);
			static void UpdateAttack(EnemyController* enemy);
			static void ExitAttack(EnemyController* enemy);
			static int CheckAttack(EnemyController* enemy);

			/** 帰巣 */
			static void EnterReturnHome(EnemyController* enemy);
			static void UpdateReturnHome(EnemyController* enemy);
			static void ExitReturnHome(EnemyController* enemy);
			static int CheckReturnHome(EnemyController* enemy);

			/** クールダウン */
			static void EnterCoolDown(EnemyController* enemy);
			static void UpdateCoolDown(EnemyController* enemy);
			static void ExitCoolDown(EnemyController* enemy);
			static int CheckCoolDown(EnemyController* enemy);

		private:
			Enemy* m_target = nullptr;
			ChildPenguin* m_foundPenguin;

			float m_elapsedTime = 0.0f;
			float m_coolDownTimer;

			/** targetの前回の位置を保持 */
			Vector3 m_prePosition = Vector3::Zero;

			/** 徘徊開始位置 */
			Vector3 m_startPosition = Vector3::Zero;

			/** 巣の位置 */
			//Vector3 m_homePosition;

			/** ペンギンを最後に見た位置 */
			Vector3 m_lastKnownPenguinPos;

			std::vector<Vector3> m_wanderingPosList;
			int m_wanderingPosListIndex = 0;

			/** 見回し（※互換性のために残しています） */
			float m_searchAngle = 0.0f;
			float m_searchSpeed = 0.0f;
			int m_searchDir = 1;

			/** 索敵時の諦めタイマー */
			float m_searchTimer = 0.0f;

			int m_maxEatCount;
			int m_eatCount;
			bool m_isFull;

			float m_attackTimer;
			float m_attackDuration;
			bool m_isAttacking;

			/** 現在の状態 */
			EnEnemyStateID m_currentState = enEnemyState_Idle;
			/** 初期化処理をしたか */
			bool m_isInitialized = false;

			bool m_isStun;
			bool m_isHomeInitialized;
			bool m_isParamInitialized;
		};
	}
}