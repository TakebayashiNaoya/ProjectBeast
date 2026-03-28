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
			 * @brief コントローラーからの入力処理
			 * @note コントローラークラスから呼び出される
			 */
			void PlayerControllerInput(const Vector3& moveDirection, bool isSneak, bool isDash, bool isJump, bool isSlide, bool isCommandToggle);
			/**
			 * @brief ダメージ処理
			 */
			virtual void Damage() override;


		private:
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_ownerDaddyPenguin;
			/** 命令を出すかどうか(トグル) */
			bool m_isCommandToggle;
			/** スニーク状態かどうか */
			bool m_isSneak;
		};
	}
}