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


			virtual core::IState* GetChangeState() override;


		public:
			/**
			 * @brief ジャンプ処理
			 */
			void Jump();


		public:
			PenguinStateMachine(PenguinBase* ownerPenguinBase);
			~PenguinStateMachine() = default;


		private:
			/** 親ペンギンのポインタ */
			PenguinBase* m_ownerPenguinBase;
			/** 現在のジャンプパワー */
			float m_currentJumpPower;
			/** ジャンプパワー */
			float m_jumpPower;
		};
	}
}

