/**
 * @file PenguinStateMachine.h
 * @brief ペンギンのステートマシン
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
			/**
			 * @brief スライド中だけ掛ける加速度の倍率を設定する
			 */
			void SetSlideAccelerationMultiplier(const float multiplier)
			{
				m_slideAccelerationMultiplier = multiplier;
			}
			/**
			 * @brief スライド中だけ掛ける加速度の倍率を取得する
			 */
			float GetSlideAccelerationMultiplier() const
			{
				return m_slideAccelerationMultiplier;
			}
			/**
			 * @brief スライド後の減速度を設定する
			 */
			void SetSlideCoastDeceleration(const float deceleration)
			{
				m_slideCoastDeceleration = deceleration;
			}
			/**
			 * @brief 勢いだけで滑っているかを取得する
			 */
			bool GetIsSlideCoast() const
			{
				return m_isSlideCoast;
			}

			/**
			 * @brief 自動スライドの判定に使う速度（倒し量と陣形倍率を掛けた値）
			 */
			struct SlideSpeeds
			{
				/** 坂で滑った場合の速度 */
				float slope;
				/** 通常の移動速度 */
				float standing;
				/** 現在の水平速度 */
				float current;
			};
			/**
			 * @brief 自動スライドの判定に使う速度を求める（陣形倍率を先に設定しておく）
			 */
			SlideSpeeds CalcSlideSpeeds(const Vector3& moveDirection, const bool isDash) const;
			/**
			 * @brief スライド中の目標速度を求める
			 */
			float CalcSlideMoveSpeed() const;
			/**
			 * @brief スライド後の減速度を求める（急な上りでは0）
			 */
			float CalcSlideCoastDeceleration() const;


			/** ステートの変更先を取得する */
			virtual core::IState* GetChangeState() override;


		public:
			/**
			 * @brief スライドの傾斜倍率を毎フレーム更新する
			 * @details 足元の地面法線と移動入力から符号つき傾斜を求めて倍率へ変換し、
			 *          法線のちらつきで速度が脈打たないよう時定数で追従させる。
			 *          SetActionInput()から呼ばれるため個別に呼ぶ必要はない。
			 */
			void UpdateSlideSlope();
			/**
			 * @brief ダメージ処理
			 */
			virtual void Damage();
			/**
			 * @brief 死亡時の処理
			 * @note PenguinDeadState::Enter()から呼ばれる。派生クラスでオーバーライドして使用する
			 */
			virtual void OnDead() {}

			/**
			 * @brief 移動・アクションの入力をまとめて設定する
			 * @param isSlideCoast 勢いだけで滑るか（isSlideが偽なら無視）
			 */
			inline void SetActionInput(const Vector3& moveDirection, bool isSneak, bool isDash, bool isSlide, bool isSlideCoast = false)
			{
				m_moveDirection = moveDirection;
				m_isSneak = isSneak;
				m_isDash = isDash; // または SetIsDash(isDash);
				m_isSlide = isSlide;
				m_isSlideCoast = isSlide && isSlideCoast;
				m_isSwimming = IsInWater();

				// 毎フレーム呼ばれるこの関数内で傾斜倍率を更新する
				UpdateSlideSlope();
			}
			/** ログ用：現在の状態名を返す（PenguinStateMachine.cpp で定義） */
			const char* GetStateNameForLog() const;

		protected:
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


		public:
			PenguinStateMachine(PenguinBase* ownerPenguinBase);
			virtual ~PenguinStateMachine() override = default;


		protected:
			/** 親ペンギンのポインタ */
			PenguinBase* m_ownerPenguinBase;
			/** スニークするかどうか */
			bool m_isSneak;
			/** スライドするかどうか */
			bool m_isSlide;
			/** 被弾したかどうか */
			bool m_isDamaged;
			/** 渦潮の中にいるかどうか */
			bool m_isInWhirlpool;
			/** スライドの傾斜による速度倍率（平滑化済み。既定 1.0f） */
			float m_slideSlopeMultiplier = 1.0f;
			/**
			 * @brief 進行方向に対する符号つき傾斜（平滑化済み）
			 * @details 下りで正、上りで負、斜面を横切ると0。値は sinθ 相当で -1〜+1。
			 */
			float m_slideSlopeSigned = 0.0f;
			/** 上り坂でのずり落ち（負の速度倍率）を許可するか。親ペンギンだけtrue */
			bool  m_isSlideBackAllowed = false;
			/** スライド中だけ掛ける加速度の倍率 */
			float m_slideAccelerationMultiplier = 1.0f;
			/** スライド後の減速度（陣形倍率を掛ける前） */
			float m_slideCoastDeceleration = 0.0f;
			/** 勢いだけで滑っているか */
			bool  m_isSlideCoast = false;
		};
	}
}