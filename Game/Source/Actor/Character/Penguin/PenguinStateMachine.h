/**
 * @file PenguinStateMachine.h
 * @brief ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "PenguinEffectStatus.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/penguin/PenguinStatus.h"



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
			/**
			 * @brief 渦潮の中にいるかどうかを設定
			 * @param isInWhirlpool 渦潮の中にいるかどうか
			 */
			inline void SetIsInWhirlpool(const bool isInWhirlpool)
			{
				m_isInWhirlpool = isInWhirlpool;
			}


			// ここに親ペンギン共通のゲッター関数を追加していく
		public:
			/**
			 * @brief ペンギンのステータスを取得
			 * @return ペンギンのステータスポインタ
			 */
			virtual const PenguinStatus* GetPenguinStatus() const = 0;
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
			/**
			 * @被弾したかどうか取得
			 */
			inline bool GetIsDamaged() const
			{
				return m_isDamaged;
			}
			/**
			 * @brief オーナーのペンギンベースを取得
			 * @return オーナーのペンギンベースポインタ
			 */
			inline PenguinBase* GetOwnerPenguinBase() const
			{
				return m_ownerPenguinBase;
			}
			/**
			 * @brief 渦潮の中にいるかどうかを取得
			 * @return 渦潮の中にいるかどうか
			 */
			inline bool GetIsInWhirlpool() const
			{
				return m_isInWhirlpool;
			}
			/**
			 * @brief エフェクトステータスを取得
			 * @return エフェクトステータスのポインタ
			 */
			PenguinEffectStatus* GetEffectStatus() const;


			/** ステートの変更先を取得する */
			virtual core::IState* GetChangeState() override;


		public:
			/**
			 * @brief ジャンプ処理
			 */
			void Jump();
			/**
			 * @brief ダメージ処理
			 */
			virtual void Damage();
			/**
			 * @brief 死亡時の処理
			 * @note PenguinDeadState::Enter()から呼ばれる。派生クラスでオーバーライドして使用する
			 */
			virtual void OnDead() {}

			inline void SetActionInput(const Vector3& moveDirection, bool isSneak, bool isDash, bool isJump, bool isSlide)
			{
				m_moveDirection = moveDirection;
				m_isSneak = isSneak;
				m_isDash = isDash; // または SetIsDash(isDash);
				m_isJump = isJump;
				m_isSlide = isSlide;
				m_isSwimming = IsInWater();
			}


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
				return  m_isSlide && height >= 0.0f;
			}
			/**
			 * @brief スライドステートに切り替えられるかどうか
			 * @return スライドステートに切り替えられるかどうか
			 */
			bool CanChangeSlidingState() const
			{
				return m_isSlide;
			}
			/**
			 * @brief スライドをキープできるかどうか
			 * @return スライドをキープできるかどうか
			 */
			bool CanKeepSlidingState() const
			{
				return m_isSlide;
			}
			/**
			 * @brief スライド終了ステートが終わったかどうか
			 * @return スライド終了ステートが終わったかどうか
			 */
			bool IsFinishedSlideEndState() const
			{
				return !IsPlayingAnimation();
			}
			/**
			 * @brief 被弾ステートに切り替えられるかどうか
			 * @return 被弾ステートに切り替えられるかどうか
			 */
			bool CanChangeDamagedState() const
			{
				return m_isDamaged;
			}
			/**
			 * @brief 泳ぎステートに切り替えられるかどうか
			 * @return 泳ぎステートに切り替えられるかどうか
			 */
			bool CanChangeSwimState() const
			{
				return IsInWater();
			}
			/**
			 * @brief 渦潮の中にいるステートに切り替えられるかどうか
			 * @return 渦潮の中にいるステートに切り替えられるかどうか
			 */
			bool CanChangeInWhirlpoolState() const
			{
				return m_isInWhirlpool;
			}


			/** ログ用：現在の状態名を返す（PenguinStateMachine.cpp で定義） */
			const char* GetStateNameForLog() const;


		public:
			PenguinStateMachine(PenguinBase* ownerPenguinBase);
			virtual ~PenguinStateMachine() override = default;


		protected:
			/** 親ペンギンのポインタ */
			PenguinBase* m_ownerPenguinBase;
			/** ジャンプパワー */
			float m_jumpPower;
			/** ジャンプするかどうか */
			bool m_isJump;
			/** スニークするかどうか */
			bool m_isSneak;
			/** スライドするかどうか */
			bool m_isSlide;
			/** 被弾したかどうか */
			bool m_isDamaged;
			/** 渦潮の中にいるかどうか */
			bool m_isInWhirlpool;
		};
	}
}