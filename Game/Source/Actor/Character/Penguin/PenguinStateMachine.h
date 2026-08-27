/**
 * @file PenguinStateMachine.h
 * @brief ペンギンのステートマシン
 */
#pragma once
#include "PenguinEffectStatus.h"
#include "PenguinStaminaGauge.h"
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
			 * @brief スライド入力中かどうかを取得
			 * @return スライド入力中かどうか
			 */
			inline bool GetIsSlide() const
			{
				return m_isSlide;
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
			 * @brief 現在水の中にいるかどうかを取得
			 * @details Update()によるステート遷移を経由せずに判定できるよう公開している
			 *          （カウントダウン中など、ステートマシンのUpdate()を呼ばずに水中判定だけ行いたい場合に使う）
			 * @return 水の中にいるかどうか
			 */
			inline bool IsInWater() const
			{
				return CharacterStateMachine::IsInWater();
			}
			/**
			 * @brief エフェクトステータスを取得
			 * @return エフェクトステータスのポインタ
			 */
			PenguinEffectStatus* GetEffectStatus() const;

			/**
			 * @brief ジャンプが使用可能かどうかを取得
			 * @return スタミナが枯渇しクールダウン中でなければtrue
			 */
			bool CanUseJump() const
			{
				return m_jumpStaminaGauge.CanUse();
			}
			/**
			 * @brief スライドが使用可能かどうかを取得
			 * @details スライドのスタミナは撤廃した（2026-08-23 試遊フィードバック）。
			 *          スライドの制限は傾斜モデル（上り坂で滑れない）が担う
			 * @return 常にtrue
			 */
			bool CanUseSlide() const
			{
				return true;
			}
			/**
			 * @brief ジャンプのスタミナゲージの割合を取得（UI表示用）
			 * @return 0.0(空)〜1.0(満タン)の割合
			 */
			float GetJumpStaminaRatio() const
			{
				return m_jumpStaminaGauge.GetRatio();
			}
			/**
			 * @brief スライドのスタミナゲージの割合を取得（UI表示用）
			 * @return 0.0(空)〜1.0(満タン)の割合
			 */
			float GetSlideStaminaRatio() const
			{
				return m_slideStaminaGauge.GetRatio();
			}

			/**
			 * @brief スライドの傾斜による速度倍率を取得する
			 * @details 下り坂で1を超え、上り坂で1を下回る。非接地・平地では1.0f。
			 *          スライド系ステートが SetMoveSpeed() に掛けて使う。
			 *          カーブと数値の根拠は docs/スライドの傾斜モデル.md を参照。
			 * @return 速度倍率（SLIDE_SLOPE_MUL_MIN 〜 SLIDE_SLOPE_MUL_MAX）
			 */
			float GetSlideSlopeMultiplier() const
			{
				return m_slideSlopeMultiplier;
			}
			/**
			 * @brief 進行方向に対する符号つき傾斜（平滑化済み）を取得する
			 * @details 下りで正、上りで負。値は sin(傾斜角) に一致する。
			 *          AI側が「上り坂ではスライドを選ばない」判断に使う
			 * @return 符号つき傾斜（-1.0〜1.0）
			 */
			float GetSlideSlopeSigned() const
			{
				return m_slideSlopeSigned;
			}
			/**
			 * @brief 上り坂でのずり落ち（負の速度倍率）を許可する
			 * @details プレイヤー操作の親ペンギンだけ有効にする。
			 *          AIの子ペンギンで有効にすると上り坂で永久に後退してしまう
			 * @param isAllowed 許可するかどうか
			 */
			void SetSlideBackAllowed(const bool isAllowed)
			{
				m_isSlideBackAllowed = isAllowed;
			}
			/**
			 * @brief 傾斜を織り込んだスライド速度を取得する
			 * @details PenguinStatus の slideSpeed に GetSlideSlopeMultiplier() を掛けたもの。
			 *          スライド系3ステート（SlideStart / Sliding / SlideEnd）が
			 *          同じ値を使うためにここへ集約している。
			 * @return スライドの移動速度
			 */
			float CalcSlideSpeedWithSlope() const;
			/**
			 * @brief スライド中の旋回速度の倍率を取得する
			 * @details 加速しているぶんだけ曲がりにくくする（1 / 速度倍率）。
			 *          上り・平地では 1.0f を返し、操作性を落とさない。
			 * @return 旋回速度の倍率
			 */
			float CalcSlideTurnMultiplier() const;


			/** ステートの変更先を取得する */
			virtual core::IState* GetChangeState() override;


		public:
			/**
			 * @brief ジャンプ処理
			 */
			void Jump();
			/**
			 * @brief ジャンプのスタミナを即座に全消費し、クールダウンに入れる
			 */
			void ConsumeJumpStamina()
			{
				m_jumpStaminaGauge.ConsumeAll();
			}
			/**
			 * @brief ジャンプ・スライドのスタミナゲージを毎フレーム更新する
			 * @details スライド側の消費速度には傾斜倍率が掛かる（下りほど減らない）。
			 */
			void UpdateStaminaGauges();
			/**
			 * @brief スライドの傾斜倍率を毎フレーム更新する
			 * @details 足元の地面法線と移動入力から符号つき傾斜を求めて倍率へ変換し、
			 *          法線のちらつきで速度が脈打たないよう時定数で追従させる。
			 *          UpdateStaminaGauges() の先頭から呼ばれるため個別に呼ぶ必要はない。
			 */
			void UpdateSlideSlope();
			/**
			 * @brief ジャンプ・スライドのスタミナゲージをPenguinStatusの値で初期化する
			 */
			void SetupStaminaGauges();
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

				// 毎フレーム呼ばれるこの関数内で、ジャンプ・スライドのスタミナゲージを更新する。
				UpdateStaminaGauges();
			}
			/** ログ用：現在の状態名を返す（PenguinStateMachine.cpp で定義） */
			const char* GetStateNameForLog() const;

		protected:
			/**
			 * @brief ジャンプステートに切り替えられるかどうか
			 * @return ジャンプステートに切り替えられるかどうか
			 */
			bool CanChangeJumpState() const
			{
				return m_isJump && IsOnGround() && CanUseJump();
			}
			/**
			 * @brief スライド開始ステートに切り替えられるかどうか
			 * @return スライド開始ステートに切り替えられるかどうか
			 */
			bool CanChangeSlideStartState() const
			{
				const float height = m_transform.m_position.y;
				return  m_isSlide && height >= 0.0f && CanUseSlide();
			}
			/**
			 * @brief スライドステートに切り替えられるかどうか
			 * @return スライドステートに切り替えられるかどうか
			 */
			bool CanChangeSlidingState() const
			{
				return m_isSlide && CanUseSlide();
			}
			/**
			 * @brief スライドをキープできるかどうか
			 * @return スライドをキープできるかどうか
			 */
			bool CanKeepSlidingState() const
			{
				return m_isSlide && CanUseSlide();
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
			/** ジャンプのスタミナ(オーバーヒート式)ゲージ */
			PenguinStaminaGauge m_jumpStaminaGauge;
			/** スライドのスタミナ(オーバーヒート式)ゲージ */
			PenguinStaminaGauge m_slideStaminaGauge;
			/** スタミナゲージの初期化が完了したかどうか */
			bool m_isStaminaGaugeSetup = false;
			/** スライドの傾斜による速度倍率（平滑化済み。既定 1.0f） */
			float m_slideSlopeMultiplier = 1.0f;
			/**
			 * @brief 進行方向に対する符号つき傾斜（平滑化済み）
			 * @details 下りで正、上りで負、斜面を横切ると0。値は sinθ 相当で -1〜+1。
			 *          スタミナ消費倍率の算出にも使うため倍率とは別に保持する。
			 */
			float m_slideSlopeSigned = 0.0f;
			/** 上り坂でのずり落ち（負の速度倍率）を許可するか。親ペンギンだけtrue */
			bool  m_isSlideBackAllowed = false;
		};
	}
}