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
			/**
			 * @brief 渦潮に移動するフラグを設定
			 * @param value 渦潮に移動するフラグ
			 */
			inline void SetIsGoingToWhirlpool(bool value) { m_isGoingToWhirlpool = value; }
			/**
			 * @brief 渦潮に移動するフラグを取得
			 * @return 渦潮に移動するフラグ
			 */
			inline bool GetIsGoingToWhirlpool() const { return m_isGoingToWhirlpool; }
			/**
			 * @brief 渦潮への到達フラグを設定
			 * @param value 渦潮への到達フラグ
			 */
			inline void SetIsAtWhirlpool(bool value) { m_isAtWhirlpool = value; }
			/**
			 * @brief 渦潮への到達フラグを取得
			 * @return 渦潮への到達フラグ
			 */
			inline bool GetIsAtWhirlpool() const { return m_isAtWhirlpool; }
			/**
			 * @brief 渦潮の座標を設定
			 * @param pos 渦潮の座標
			 */
			inline void SetWhirlpoolTargetPos(const Vector3& pos) { m_whirlpoolTargetPos = pos; }
			/**
			 * @brief 渦潮の座標を取得
			 * @return 渦潮の座標
			 */
			inline const Vector3& GetWhirlpoolTargetPos() const { return m_whirlpoolTargetPos; }
			/**
			 * @brief 渦潮への飛び込みが完了したことをAIに伝えるフラグを設定
			 * @param value 渦潮への飛び込みが完了したことをAIに伝えるフラグ
			 */
			inline void SetHasFinishedDiving(bool value) { m_hasFinishedDiving = value; }
			/**
			 * @brief 渦潮への飛び込みが完了したことをAIに伝えるフラグを取得
			 * @return 渦潮への飛び込みが完了したことをAIに伝えるフラグ
			 */
			inline bool GetHasFinishedDiving() const { return m_hasFinishedDiving; }


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

			/** 渦潮に移動するフラグ */
			bool m_isGoingToWhirlpool = false;
			/** 渦潮への到達フラグ */
			bool m_isAtWhirlpool = false;
			/** 渦潮の座標 */
			Vector3 m_whirlpoolTargetPos = Vector3::Zero;
			/** 渦潮への飛び込みが完了したことをAIに伝えるフラグ */
			bool m_hasFinishedDiving = false;
		};
	}
}