/**
 * @file NaughtyChildPenguinStateMachine.h
 * @brief ヤンチャペンギンのステートマシン
 * @author 立山
 */
#pragma once
#include "ChildPenguinStateMachine.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;
		class Enemy;

		class NaughtyChildPenguinStateMachine : public ChildPenguinStateMachine
		{
		public:
			inline ChildPenguin* GetOwnerChildPenguin() const { return m_ownerChildPenguin; }

			/**
			 * @brief シロクマを起こしに行くフラグを設定
			 * @param value シロクマを起こしに行くフラグ
			 */
			inline void SetIsGoingToWakeBear(bool value) { m_isGoingToWakeBear = value; }
			/**
			 * @brief シロクマを起こしに行くフラグを取得
			 * @return シロクマを起こしに行くフラグ
			 */
			inline bool GetIsGoingToWakeBear() const { return m_isGoingToWakeBear; }
			/**
			 * @brief シロクマへの到達フラグを設定
			 * @param value シロクマへの到達フラグ
			 */
			inline void SetIsAtBear(bool value) { m_isAtBear = value; }
			/**
			 * @brief シロクマへの到達フラグを取得
			 * @return シロクマへの到達フラグ
			 */
			inline bool GetIsAtBear() const { return m_isAtBear; }
			/**
			 * @brief シロクマの座標を設定
			 * @param pos シロクマの座標
			 */
			inline void SetBearTargetPos(const Vector3& pos) { m_bearTargetPos = pos; }
			/**
			 * @brief シロクマの座標を取得
			 * @return シロクマの座標
			 */
			inline const Vector3& GetBearTargetPos() const { return m_bearTargetPos; }
			/**
			 * @brief 起こしに行くシロクマを設定
			 * @param bear シロクマのポインタ
			 */
			inline void SetTargetBear(Enemy* bear) { m_targetBear = bear; }
			/**
			 * @brief 起こしに行くシロクマを取得
			 * @return 起こしに行くシロクマのポインタ
			 */
			inline Enemy* GetTargetBear() const { return m_targetBear; }
			/**
			 * @brief シロクマが起きたかどうかをAIに伝えるフラグを設定
			 * @param value シロクマが起きたかどうかをAIに伝えるフラグ
			 */
			inline void SetHasFinishedWaking(bool value) { m_hasFinishedWaking = value; }
			/**
			 * @brief シロクマが起きたかどうかをAIに伝えるフラグを取得
			 * @return シロクマが起きたかどうかをAIに伝えるフラグ
			 */
			inline bool GetHasFinishedWaking() const { return m_hasFinishedWaking; }

		public:
			NaughtyChildPenguinStateMachine(ChildPenguin* ownerChildPenguin);
			~NaughtyChildPenguinStateMachine() = default;

		protected:
			core::IState* GetTypeSpecificChangeState() override;

		private:
			ChildPenguin* m_ownerChildPenguin;

			/** シロクマを起こしに行くフラグ（AIがtrueにし、WakeBear終了時にリセット） */
			bool m_isGoingToWakeBear;
			/** シロクマへの到達フラグ（AIがtrueにし、WakeBear終了時にリセット） */
			bool m_isAtBear;
			/** シロクマの座標（AIがセット） */
			Vector3 m_bearTargetPos;

			/** 起こしに行くシロクマのポインタ*/
			Enemy* m_targetBear;

			/** アニメーションが完了したことをAIに伝えるフラグ */
			bool m_hasFinishedWaking = false;
		};
	}
}