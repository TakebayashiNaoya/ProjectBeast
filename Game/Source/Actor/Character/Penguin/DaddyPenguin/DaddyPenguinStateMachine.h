/**
 * @file DaddyPenguinStateMachine.h
 * @brief 親ペンギンのステートマシン
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class DaddyPenguin;
		class DaddyPenguinStatus;


		/**
		 * @brief 親ペンギンのステートマシンクラス
		 */
		class DaddyPenguinStateMachine : public PenguinStateMachine
		{
			// ここに親ペンギン固有のセッター関数を追加していく
		public:
			/**
			 * @brief 命令（Yボタン）を出すかどうかを設定
			 * @param isCommandToggle 命令を出すかどうか
			 * @note Yボタンの中身は「待機・追従の切り替え」から
			 *       「散った子ペンギンの再集合を呼びかける」へ変わっている
			 *       （DaddyPenguinCommandShoutState::Enter()）。
			 *       ボタンから鳴き声ステートへ繋ぐ配線としては役割が変わっていないため、
			 *       名前はそのままにしてある。
			 */
			inline void SetIsCommandToggle(const bool isCommandToggle)
			{
				m_isCommandToggle = isCommandToggle;
			}
			/**
			 * @brief 勝ったかどうかを設定
			 * @param isWin 勝ったかどうか
			 */
			inline void SetIsWin(const bool isWin)
			{
				m_isWin = isWin;
			}
			/**
			 * @brief 負けたかどうかを設定
			 * @param isLose 負けたかどうか
			 */
			inline void SetIsLose(const bool isLose)
			{
				m_isLose = isLose;
			}
			/**
			 * @brief かまくらに入るかどうかを設定
			 */
			inline void SetIsEnterIgloo(const bool isEnterIgloo)
			{
				m_isEnterIgloo = isEnterIgloo;
			}
			/**
			 * @brief イベント中かどうか
			 */
			inline void SetIsIglooEventFinished(const bool isFinished)
			{
				m_isIglooEventFinished = isFinished;
			}
			/**
			 * @brief かまくら内にいるかどうかを設定
			 */
			inline void SetIsInsideIgloo(const bool isInside)
			{
				m_isInsideIgloo = isInside;
			}


			// ここに親ペンギン固有のゲッター関数を追加していく
		public:
			/**
			 * @brief 命令（トグル）を出すかどうかを取得
			 * @return 命令を出すかどうか
			 */
			inline bool GetIsCommandToggle() const
			{
				return m_isCommandToggle;
			}
			/**
			 * @brief 命令が出ているか
			 * @return 命令が出ているか
			 */
			inline bool IsCommandState() const
			{
				return CanChangeCommandState();
			}
			/**
			 * @brief かまくらに入るかどうかを取得
			 */
			inline bool GetIsEnterIgloo() const
			{
				return m_isEnterIgloo;
			}
			/**
			 * @brief イベント中かどうかの取得
			 */
			inline bool GetIsIglooEventFinished() const
			{
				return m_isIglooEventFinished;
			}
			/**
			 * @brief かまくら内にいるかどうかを取得
			 */
			inline bool GetIsInsideIgloo() const
			{
				return m_isInsideIgloo;
			}
			/**
			 * @brief 親ペンギンのステータスを取得
			 * @return 親ペンギンのステータスポインタ
			 */
			DaddyPenguinStatus* GetDaddyPenguinStatus() const;
			/**
			 * @brief ペンギンのステータスを取得（基底クラスのオーバーライド）
			 * @return ペンギンのステータスポインタ
			 */
			virtual const PenguinStatus* GetPenguinStatus() const override;

			/**
			 * @brief 親ペンギンのポインタを取得
			 * @return 親ペンギンのポインタ
			 */
			DaddyPenguin* GetOwnerDaddyPenguin() const
			{
				return m_ownerDaddyPenguin;
			}


			/** ステートの変更先を取得する */
			virtual core::IState* GetChangeState() override;


		public:
			DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin);
			~DaddyPenguinStateMachine() = default;


		private:
			/**
			 * @brief 命令が出ているか
			 * @return 命令が出ているか
			 */
			bool CanChangeCommandState() const
			{
				return m_isCommandToggle;
			}

			core::IState* CheckSystemState();
			core::IState* CheckCommandState();
			core::IState* CheckActionState();
			core::IState* CheckEventState();


		private:
			bool CanChangeEnterIglooState() const;


		public:
			/**
			 * @brief ダメージ処理
			 */
			virtual void Damage() override;


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_ownerDaddyPenguin;
			/** 命令を出すかどうか(トグル) */
			bool m_isCommandToggle;
			/** 勝ったかどうか */
			bool m_isWin;
			/** 負けたかどうか */
			bool m_isLose;
			/** かまくらに入るかどうか */
			bool m_isEnterIgloo;
			/** イベントが完了したか */
			bool m_isIglooEventFinished;
			/** かまくら内にいるかどうか */
			bool m_isInsideIgloo;
		};
	}
}