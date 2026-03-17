/**
 * @file PenguinStateMachine.h
 * @brief ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class PenguinBase;

		/**
		 * @brief ペンギンのステートマシンクラス
		 */
		class PenguinStateMachine : public CharacterStateMachine
		{
			// ここにペンギン共通のセッター関数を追加していく
		public:
			/**
			 * @brief ジャンプパワーを設定
			 * @param jumpPower ジャンプパワー
			 */
			inline void SetJumpPower(const float jumpPower)
			{
				m_jumpPower = jumpPower;
			}
			/**
			 * @brief ジャンプするかどうかを設定
			 * @param isJump ジャンプするかどうか
			 */
			inline void SetIsJump(const bool isJump)
			{
				m_isJump = isJump;
			}
			/**
			 * @brief 滞空時間を設定
			 */
			inline void SetAirTime(const float airTime)
			{
				m_airTime = airTime;
			}


			// ここに親ペンギン共通のゲッター関数を追加していく
		public:
			/**
			 * @brief ジャンプパワーを取得
			 * @return ジャンプパワー
			 */
			inline float GetJumpPower() const
			{
				return m_jumpPower;
			}
			/**
			 * @brief ジャンプするかどうかを取得
			 * @return ジャンプするかどうか
			 */
			inline bool GetIsJump() const
			{
				return m_isJump;
			}


			/** ステートの変更先を取得する */
			virtual core::IState* GetChangeState() override;


		public:
			/**
			 * @brief ジャンプ処理
			 */
			void Jump();


		protected:
			/**
			 * @brief ジャンプステートに切り替えられるかどうか
			 * @return ジャンプステートに切り替えられるかどうか
			 */
			bool CanChangeJumpState() const
			{
				return m_isJump && IsOnGround();
			}


		public:
			PenguinStateMachine(PenguinBase* ownerPenguinBase);
			~PenguinStateMachine() = default;


		protected:
			/** 親ペンギンのポインタ */
			PenguinBase* m_ownerPenguinBase;
			/** 滞空時間 */
			float m_airTime;
			/** ジャンプパワー */
			float m_jumpPower;
			/**ジャンプするかどうか */
			bool m_isJump;
		};
	}
}

