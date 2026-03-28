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
			 * @brief スニーク状態かどうかを設定
			 * @param isSneak スニーク状態かどうか
			 */
			inline void SetIsSneak(const bool isSneak)
			{
				m_isSneak = isSneak;
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
			 * @brief スニーク状態かどうかを取得
			 * @return スニーク状態かどうか
			 */
			inline bool GetIsSneak() const
			{
				return m_isSneak;
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
			core::IState* GetChangeState();


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


		public:
			/**
			 * @brief プレイヤーコントローラーの入力処理
			 * @note 後にプレイヤーコントローラーに処理を移す
			 */
			void PlayerControllerInput();
			/**
			 * @brief ダメージ処理
			 */
			void Damage() override;


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_ownerDaddyPenguin;
			/** 命令を出すかどうか(トグル) */
			bool m_isCommandToggle;
			/** スニーク状態かどうか */
			bool m_isSneak;
			/** 勝ったかどうか */
			bool m_isWin;
			/** 負けたかどうか */
			bool m_isLose;
		};
	}
}