/**
 * @file ChildPenguinStateMachine.h
 * @brief 子ペンギンのステートマシン
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStatus;
		class Enemy;


		/**
		 * @brief 子ペンギンのステートマシンクラス
		 */
		class ChildPenguinStateMachine : public PenguinStateMachine
		{
		public:
			/**
			 * @brief 子ペンギンのステータスを取得
			 * @return 子ペンギンのステータスポインタ
			 */
			const ChildPenguinStatus* GetChildPenguinStatus() const;

			/**
			 * @brief ペンギンのステータスを取得（基底クラスのオーバーライド）
			 * @return ペンギンのステータスポインタ
			 */
			virtual const PenguinStatus* GetPenguinStatus() const override;

			/** ステートの変更先を取得する */
			core::IState* GetChangeState() override;


		public:
			/**
			 * @brief ダメージ処理
			 */
			void Damage() override;
			/**
			 * @brief 死亡時の処理
			 * @note PenguinDeadState::Enter()から呼ばれる
			 * @note ChildPenguinManagerからの削除とdeleteを行う
			 */
			void OnDead() override;


		public:
			ChildPenguinStateMachine(ChildPenguin* ownerChildPenguin, EnChildPenguinType type);
			~ChildPenguinStateMachine() = default;

			/**
			 * @brief 死亡終了後にステートマシンを終了させるかどうか
			 * @param finish trueなら死亡終了後にステートマシンを終了させる
			 */
			void SetFinishAfterDeath(bool finish) { m_isFinishAfterDeath = finish; }

			/**
			 * @brief 死亡終了したかどうか
			 * @return 死亡終了したかどうか
			 */
			bool IsFinishAfterDeath() const { return m_isFinishAfterDeath; }

			/**
			 * @brief 攻撃したものを設定
			 * @param enemy 攻撃者のポインタ
			 */
			void SetAttackerEnemy(Enemy* enemy) { m_attackerEnemy = enemy; }

			/**
			 * @brief 攻撃者を取得
			 * @return 攻撃者
			 */
			Enemy* GetAttackerEnemy() const { return m_attackerEnemy; }


		protected:
			/**
			 * @brief タイプ固有のステート遷移
			 * @note 固有ステートへの遷移が必要な派生クラスでオーバーライドする
			 * @return 遷移先ステート。遷移不要ならnullptr
			 */
			virtual core::IState* GetTypeSpecificChangeState() { return nullptr; }


		private:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_ownerChildPenguin;
			/** タイプを保持 */
			EnChildPenguinType m_type;
			/** 死亡終了時 */
			bool m_isFinishAfterDeath;
			/** 攻撃者 */
			Enemy* m_attackerEnemy;
		};
	}
}