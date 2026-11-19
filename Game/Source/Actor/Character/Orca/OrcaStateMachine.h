/**
 * @file OrcaStateMachine.h
 * @brief シャチのステートマシン
 */
#pragma once
#include "OrcaTypes.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Orca;
		class OrcaStatus;
		class OrcaSeaGrid;
		struct MasterOrcaParameter;


		/**
		 * @brief シャチのステートマシン
		 * @details 判断は OrcaController が行い、ここは「どう動くか」だけを受け持つ（EnemyStateMachine と同じ分担）。
		 *          フラグの名前は EnemyStateMachine に合わせている（UI 側がクマと同じ読み方をできるようにするため）。
		 *          XZ を書き換えるのは MoveInSea() と ApplyKnockback() だけで、どちらも OrcaSeaGrid::ResolveMove() の戻り値しか書かない。
		 *          物理ボディ（CharacterController）は使わない。
		 */
		class OrcaStateMachine : public CharacterStateMachine
		{
		public:
			/**
			 * @brief 1フレーム分、海の中を泳がせる
			 * @details 向きを移動方向へ turnSpeedDeg で回し、向いている方へ進む。目標との角度が大きいほど減速する。
			 *          Y は「波面 − 深さ」にする
			 */
			void MoveInSea();

			/**
			 * @brief 水平に弾き飛ばす（ResolveMove を通すので海からは出ない）
			 * @param direction 弾く向き（Y は無視）
			 * @param distance 弾く距離
			 */
			void ApplyKnockback(const Vector3& direction, float distance);

			/**
			 * @brief 向きを即座に設定する（出現時など）
			 * @param direction 向き（Y は無視）
			 */
			void SetFacing(const Vector3& direction);

			/** @brief 体の正面（水平・正規化済み） */
			Vector3 GetForward() const;

			/**
			 * @brief 出現時の姿勢（大きさ・深さ・向き）を決める
			 * @details カウントダウン中は AI もステートマシンも動かず、モデルの行列だけが更新される。
			 *          そのあいだ原寸・水面の高さで見えないよう、出現した時点で決めておく
			 */
			void InitializePose();


		public:
			/** @brief 泳ぐ深さの段階を設定する */
			inline void SetDepthLevel(EnOrcaDepthLevel level) { m_depthLevel = level; }
			/** @brief 現在の深さ（波面からの下がり量） */
			inline float GetDepth() const { return m_depth; }
			/** @brief 現在の速さ */
			inline float GetCurrentSpeed() const { return m_currentSpeed; }
			/** @brief 波面の高さ（真上の水面のY） */
			float GetWaveY() const { return CalcCurrentWaveY(); }
			/** @brief 真上の水面の点（UI・音・視界の基準） */
			Vector3 GetSurfacePosition() const;
			/** @brief 口先の点（Y は水面。攻撃距離・子の逃げる基準） */
			Vector3 GetMouthPosition() const;

			/** @brief 泳げる場所の地図を設定する */
			inline void SetSeaGrid(const OrcaSeaGrid* grid) { m_seaGrid = grid; }
			/** @brief 泳げる場所の地図 */
			inline const OrcaSeaGrid* GetSeaGrid() const { return m_seaGrid; }

			/** @brief パラメーター（未読み込みなら nullptr） */
			const MasterOrcaParameter* GetParameter() const;
			/** @brief ステータス */
			OrcaStatus* GetOwnerStatus() const;


		public:
			// --- EnemyStateMachine と同名のフラグ ---
			inline bool IsSearch() const { return m_isSearch; }
			inline void SetSearch(bool value) { m_isSearch = value; }
			/** @brief 追跡・巡回をやめて巡回をやり直すよう求められたか（クマの IsReturnHome に当たる。シャチは帰巣しない） */
			inline bool IsGiveUpRequested() const { return m_isGiveUpRequested; }
			inline void SetGiveUpRequested(bool value) { m_isGiveUpRequested = value; }
			inline bool IsChasing() const { return m_isChasing; }
			inline void SetIsChasing(bool value) { m_isChasing = value; }
			inline bool IsAttack() const { return m_isAttack; }
			inline void SetAttack(bool value) { m_isAttack = value; }
			inline bool IsRoar() const { return m_isRoar; }
			inline void SetRoar(bool value) { m_isRoar = value; }
			inline bool IsStun() const { return m_isStun; }
			inline void SetStun(bool value) { m_isStun = value; }
			inline const Vector3& GetSearchTargetPos() const { return m_searchTargetPos; }
			inline void SetSearchTargetPos(const Vector3& pos) { m_searchTargetPos = pos; }
			/** @brief 噛みついた瞬間（attackImpactTime 経過）か。読んだ側がリセットする（EnemyStateMachine と同じ） */
			inline bool IsAttackImpact() const { return m_isAttackImpact; }
			inline void SetAttackImpact(bool value) { m_isAttackImpact = value; }
			/** @brief 攻撃のアニメが終わったか */
			inline bool IsAttackFinished() const { return m_isAttackFinished; }
			inline void SetAttackFinished(bool value) { m_isAttackFinished = value; }

			/** @brief ログに書く状態名（語は EnemyStateMachine::GetStateNameForLog に合わせる） */
			const char* GetStateNameForLog() const;
			/** @brief ログに書く状態名を設定する（OrcaController が状態を変えるたびに呼ぶ） */
			inline void SetStateNameForLog(const char* name) { m_stateNameForLog = name; }


		public:
			core::IState* GetChangeState() override;


		public:
			OrcaStateMachine(Orca* owner);
			~OrcaStateMachine() override;


		private:
			/** @brief 深さの段階 → 深さ */
			float GetTargetDepth() const;
			/** @brief 深さの段階 → 頭の持ち上げ角度（ラジアン） */
			float GetTargetPitch() const;
			/** @brief 深さと頭の角度を、イーズインアウトで目標へ近づける */
			void UpdateDepthAndPitch(float deltaTime);
			/** @brief 向き（左右の角度と、頭の持ち上げ角度）から回転を作る */
			void ApplyRotation();


		private:
			Orca* m_owner = nullptr;
			const OrcaSeaGrid* m_seaGrid = nullptr;

			/** 体の向き（Y軸回りの角度。ラジアン） */
			float m_yaw = 0.0f;
			/** 頭の持ち上げ角度（ラジアン。噛みつくときだけ上を向く） */
			float m_pitch = 0.0f;
			/** 現在の速さ */
			float m_currentSpeed = 0.0f;
			/** 現在の深さ */
			float m_depth = 0.0f;
			/** 深さの段階（OrcaController が設定する目標） */
			EnOrcaDepthLevel m_depthLevel = EnOrcaDepthLevel::Swim;
			/** 深さを最初に決めたか（出現時は補間せず目標の深さから始める） */
			bool m_isDepthInitialized = false;

			/**
			 * 深さと頭の角度の移り変わり（イーズインアウト）
			 * @details 段階が変わったら、そのときの値から新しい段階の値へ、決まった時間で滑らかに移す
			 */
			EnOrcaDepthLevel m_transitionLevel = EnOrcaDepthLevel::Swim;
			float m_depthFrom = 0.0f;
			float m_pitchFrom = 0.0f;
			float m_transitionElapsed = 0.0f;
			float m_transitionDuration = 0.0f;

			bool m_isSearch = false;
			bool m_isGiveUpRequested = false;
			bool m_isChasing = false;
			bool m_isAttack = false;
			bool m_isRoar = false;
			bool m_isStun = false;
			bool m_isAttackImpact = false;
			bool m_isAttackFinished = false;
			Vector3 m_searchTargetPos = Vector3::Zero;
			const char* m_stateNameForLog = "Idle";

		};
	}
}
