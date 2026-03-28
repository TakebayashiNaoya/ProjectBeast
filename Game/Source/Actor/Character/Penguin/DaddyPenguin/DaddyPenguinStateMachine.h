/**
 * @file DaddyPenguinStateMachine.h
 * @brief 親ペンギンのステートマシン
 * @author 藤谷
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
			 * @brief 命令（トグル）を出すかどうかを設定
			 * @param isCommandToggle 命令を出すかどうか
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
			 * @brief 親ペンギンのステータスを取得
			 * @return 親ペンギンのステータスポインタ
			 */
			DaddyPenguinStatus* GetDaddyPenguinStatus() const;
			/**
			 * @brief ペンギンのステータスを取得（基底クラスのオーバーライド）
			 * @return ペンギンのステータスポインタ
			 */
			virtual const PenguinStatus* GetPenguinStatus() const override;


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
		};
	}
}