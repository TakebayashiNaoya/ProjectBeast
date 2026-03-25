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
			 * @brief 追従命令を出すかどうかを設定
			 * @param isFollowCommand 追従命令を出すかどうか
			 */
			inline void SetIsFollowCommand(const bool isFollowCommand)
			{
				m_isFollowCommand = isFollowCommand;
			}
			/**
			 * @brief 待機命令を出すかどうかを設定
			 * @param isWaitCommand 待機命令を出すかどうか
			 */
			inline void SetIsWaitCommand(const bool isWaitCommand)
			{
				m_isWaitCommand = isWaitCommand;
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
			 * @brief 追従命令を出すかどうかを取得
			 * @return 追従命令を出すかどうか
			 */
			inline bool GetIsFollowCommand() const
			{
				return m_isFollowCommand;
			}
			/**
			 * @brief 待機命令を出すかどうかを取得
			 * @return 待機命令を出すかどうか
			 */
			inline bool GetIsWaitCommand() const
			{
				return m_isWaitCommand;
			}
			/**
			 * @brief 追従命令か待機命令が出ているか
			 * @return 追従命令か待機命令が出ているか
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


			/** ステートの変更先を取得する */
			core::IState* GetChangeState();


		public:
			DaddyPenguinStateMachine(DaddyPenguin* ownerDaddyPenguin);
			~DaddyPenguinStateMachine() = default;


		private:
			/**
			 * @brief 追従命令か待機命令が出ているか
			 * @return 追従命令か待機命令が出ているか
			 */
			bool CanChangeCommandState() const
			{
				return m_isFollowCommand || m_isWaitCommand;
			}


		public:
			/**
			 * @brief プレイヤーコントローラーの入力処理
			 * @note 後にプレイヤーコントローラーに処理を移す
			 */
			void PlayerControllerInput();


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_ownerDaddyPenguin;
			/** 追従命令を出すかどうか */
			bool m_isFollowCommand;
			/** 待機命令を出すかどうか */
			bool m_isWaitCommand;
			/** 勝ったかどうか */
			bool m_isWin;
			/** 負けたかどうか */
			bool m_isLose;
		};
	}
}

