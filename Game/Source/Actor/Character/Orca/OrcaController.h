/**
 * @file OrcaController.h
 * @brief シャチのAI
 */
#pragma once
#include <vector>
#include "Source/Actor/Character/PlayerFront.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Orca;
		class ChildPenguin;
		class OrcaSeaGrid;
		class OrcaStateMachine;
		struct MasterOrcaParameter;


		/**
		 * @brief シャチのAI
		 * @details EnemyController（シロクマのAI）の状態と遷移を1対1で写している。
		 *          差し替えているのは、移動（海の地図と経路）、標的の条件（泳いでいる子だけ）、アニメと深さ。
		 *          クマが帰巣して眠る場面では、シャチは巡回へ戻る（シャチは巣と睡眠を持たない）。
		 *          各状態の関数には、対応する EnemyController.cpp の行番号をコメントで残している
		 *          （クマのAIを直したときに、こちらへ手で反映するための手がかり）。
		 */
		class OrcaController : public Noncopyable
		{
		public:
			/** AIの状態（EnemyController::EnEnemyStateID のうち、シャチで使うもの） */
			enum class EnState : uint8_t
			{
				Idle,
				Search,
				Wandering,
				Roar,
				Chase,
				Attack,
				Stun,
				Num
			};


		public:
			OrcaController();
			~OrcaController();

			void Update();


		public:
			/** @brief 操作対象を設定する */
			void SetTarget(Orca* target) { m_target = target; }
			/** @brief 巡回点を足す */
			void AddPatrolPoint(const Vector3& point) { m_patrolPoints.push_back(point); }
			/** @brief 巡回点 */
			const std::vector<Vector3>& GetPatrolPoints() const { return m_patrolPoints; }

			/** @brief 追っている子（いなければ nullptr） */
			const ChildPenguin* GetFoundPenguin() const { return m_foundPenguin; }
			/** @brief 現在の状態 */
			EnState GetState() const { return m_state; }
			/** @brief 状態の名前（ログ・デバッグ表示用。語は EnemyStateMachine::GetStateNameForLog に合わせる） */
			static const char* GetStateName(EnState state);
			/** @brief いま向かっている経路の経由点の数（デバッグ表示用） */
			int GetPathPointCount() const { return static_cast<int>(m_path.size()); }
			/** @brief 食べた数（デバッグ表示用） */
			int GetEatCount() const { return m_eatCount; }

			/**
			 * @brief 噛んだ子の幽霊が消えた（幽霊デバフ）
			 * @details クマはここで帰巣するが、シャチは追跡・巡回をやめて巡回をやり直す（スタミナと満腹も戻す）
			 */
			void OnVictimGhostHidden();


		public:
			// --- デバッグ欄のボタン用 ---
			/** @brief 幽霊デバフと同じことを今すぐ起こす（巡回へ戻る） */
			void DebugTriggerGhostDebuff() { OnVictimGhostHidden(); }
			/** @brief スタミナを0にする */
			void DebugDrainStamina();
			/** @brief 最も近い陸の方向へ強く弾く（海から出ないことの確認用） */
			void DebugKnockbackTowardLand();


		private:
			/** @brief 状態を切り替える（Exit → Enter） */
			void ChangeState(EnState next);

			void EnterState(EnState state);
			void UpdateState(EnState state);
			void ExitState(EnState state);
			/** @return 次の状態。切り替えないなら Num */
			EnState CheckState(EnState state);

			// --- 各状態（EnemyController.cpp の対応箇所をコメントに書く） ---
			void EnterIdle();
			void UpdateIdle();
			EnState CheckIdle();

			void EnterSearch();
			void UpdateSearch();
			void ExitSearch();
			EnState CheckSearch();

			void EnterWandering();
			void UpdateWandering();
			void ExitWandering();
			EnState CheckWandering();

			void EnterRoar();
			void UpdateRoar();
			void ExitRoar();
			EnState CheckRoar();

			void EnterChase();
			void UpdateChase();
			void ExitChase();
			EnState CheckChase();

			void EnterAttack();
			void UpdateAttack();
			void ExitAttack();
			EnState CheckAttack();

			void EnterStun();
			EnState CheckStun();

			/**
			 * @brief 追跡・巡回をやめて巡回をやり直す（クマなら帰巣する場面）
			 * @details スタミナを全回復し、食べた数を0に戻す。ログに理由を残す
			 * @param reason ログに残す理由（debuff / full / stamina / lost）
			 */
			EnState GiveUpAndPatrol(const char* reason);

			/**
			 * @brief 正面への回り込みを1フレーム進める（CheckChaseの先頭で、現在の標的に対して呼ぶ。EnemyControllerと同じ）
			 * @details 結果の状態とsteerPointはm_approachに入る。UpdateChaseはそれをSteerToに渡すだけ
			 */
			void EvaluateFrontApproach();
			/** @brief 回り込み中なら終えて、ログを残す */
			void EndFrontApproach(PlayerFront::EnApproachPhase nextPhase, const char* reason);
			/** @brief 攻撃してよいか（回り込み中でなく、画面に映っている） */
			bool CanAttackFromFront() const;


		private:
			/**
			 * @brief 泳いでいる子を視界で探す（EnemyController::FindTarget :266-322と同じ判定に、泳いでいるかの条件を足したもの）
			 * @details 視線は水面の高さ + 少しの位置から水平に飛ばす（水面下から飛ばすと地形の下をくぐって流氷の向こうが見えるため）
			 */
			ChildPenguin* FindTarget() const;
			/** @brief 子がまだ標的にできるか（一覧に残っている・生きている・泳いでいる） */
			bool IsTargetValid(const ChildPenguin* penguin) const;
			/** @brief 音を聞く。しきい値以上なら Search のフラグを立てる（EnemyIdleState/EnemyWalkState::Update と同じ） */
			void ListenNoise();


		private:
			/**
			 * @brief goal へ向かうように移動方向と速さを設定する
			 * @details goal が航行可能域の外なら、同じ連結域の最寄りの点を目標にする。
			 *          見通しがあれば直進し、無ければ経路を探してそれに沿う
			 * @param goal 目標
			 * @param speed 速さ
			 * @return 実際に目標にした点までの水平距離。目標が見つからなければ負の値
			 */
			float SteerTo(const Vector3& goal, float speed);
			/** @brief 止まる */
			void StopMove();
			/** @brief 止まって point の方を向く */
			void FaceTo(const Vector3& point);
			/** @brief 停滞を数える（クマの STUCK_* と同じ考え方） */
			void UpdateStuckTimer();
			/** @brief 停滞の計測をやり直す */
			void ResetStuckTimer();

			OrcaStateMachine* GetStateMachine() const;
			const MasterOrcaParameter* GetParameter() const;
			const OrcaSeaGrid* GetSeaGrid() const;


		private:
			Orca* m_target = nullptr;
			ChildPenguin* m_foundPenguin = nullptr;
			/** 子を最後に見た位置 */
			Vector3 m_lastKnownPenguinPos = Vector3::Zero;

			EnState m_state = EnState::Idle;
			bool m_isInitialized = false;

			/** 状態に入ってからの時間 */
			float m_elapsedTime = 0.0f;
			/** Idle で待つ時間（入るたびに乱数で決める） */
			float m_idleTime = 0.0f;
			/** 探索を諦めるまでの時間 */
			float m_searchTimer = 0.0f;

			/** 食べた数と満腹 */
			int m_eatCount = 0;
			bool m_isFull = false;


			/** 巡回点 */
			std::vector<Vector3> m_patrolPoints;
			int m_patrolIndex = 0;

			/** 経路 */
			std::vector<Vector3> m_path;
			int m_pathIndex = 0;
			/** 経路を探したときの目標 */
			Vector3 m_pathGoal = Vector3::Zero;
			/** 次に経路を探せるまでの時間 */
			float m_repathTimer = 0.0f;

			/** 停滞の検出 */
			Vector3 m_lastCheckPosition = Vector3::Zero;
			float m_stuckTimer = 0.0f;

			/** 正面への回り込みの状態（追跡に入るたびに初期化する） */
			PlayerFront::ApproachState m_approach;
		};
	}
}
