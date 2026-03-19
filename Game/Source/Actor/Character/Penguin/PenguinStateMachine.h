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
			 * @brief スライドするかどうかを設定
			 * @param isSlide スライドするかどうか
			 */
			inline void SetIsSlide(const bool isSlide)
			{
				m_isSlide = isSlide;
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
			/**
			 * @brief スライド開始ステートに切り替えられるかどうか
			 * @return スライド開始ステートに切り替えられるかどうか
			 */
			bool CanChangeSlideStartState() const
			{
				const float height = m_transform.m_position.y;
				return  m_isSlide /*&& IsOnGround()*/ && height >= 0.0f;
			}
			/**
			 * @brief スライドステートに切り替えられるかどうか
			 * @return スライドステートに切り替えられるかどうか
			 */
			bool CanChangeSlidingState() const
			{
				return m_isSlide && !IsPlayingAnimation();
			}
			/**
			 * @brief スライドステートを維持できるかどうか
			 * @return スライドステートを維持できるかどうか
			 */
			bool CanKeepSlidingState() const
			{
				return m_isSlide && CanChangeWalkState();
			}
			/**
			 * @brief スライド終了ステートに切り替えられるかどうか
			 * @return スライド終了ステートに切り替えられるかどうか
			 */
			bool IsFinishedSlideEndState() const
			{
				return !m_isSlide && !IsPlayingAnimation();
			}
			/**
			 * @brief 離水ステートに切り替えられるかどうか
			 * @return 離水ステートに切り替えられるかどうか
			 */
			bool CanChangeSeparateWaterState() const
			{
				return m_isSeparateWater && IsInWater();
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
			/** スライドするかどうか */
			bool m_isSlide;
			/** 離水するかどうか */
			bool m_isSeparateWater;
		};
	}
}

