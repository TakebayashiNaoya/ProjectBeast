/**
 * @file CharacterStateMachine.h
 * @brief キャラクターのステートマシン
 * @author 藤谷
 */
#pragma once
#include "CharacterBase.h"
#include "CharacterStatus.h"
#include "Source/Actor/ActorStateMachine.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			/** 海面の基準高さ */
			constexpr float SEA_LEVEL = 0.0f;
			/** 海面付近のしきい値（ゆらゆら中にSwimmingStateを維持するための余裕） */
			constexpr float SEA_SURFACE_THRESHOLD = 1.0f;
		}

		/**
		 * @brief キャラクターのステートマシン
		 */
		class CharacterStateMachine : public ActorStateMachine
		{
		public:
			/**
			 * @brief 移動方向を取得
			 * @return 移動方向
			 */
			inline const Vector3& GetMoveDirection() const
			{
				return m_moveDirection;
			}
			/**
			 * @brief 移動速度を取得
			 * @return 移動速度
			 */
			inline float GetMoveSpeed() const
			{
				return m_moveSpeed;
			}
			/**
			 * @brief ダッシュしているかどうかを取得
			 * @return ダッシュしているかどうか
			 */
			inline bool GetIsDash() const
			{
				return m_isDash;
			}


		public:
			/**
			 * @brief 移動方向を設定
			 * @param moveDirection 移動方向
			 */
			inline void SetMoveDirection(const Vector3& moveDirection)
			{
				m_moveDirection = moveDirection;
			}
			/**
			 * @brief 移動速度を設定
			 * @param moveSpeed 移動速度
			 */
			inline void SetMoveSpeed(const float moveSpeed)
			{
				m_moveSpeed = moveSpeed;
			}
			/**
			 * @brief ダッシュしているかどうかを設定
			 * @param isDash ダッシュしているかどうか
			 */
			inline void SetIsDash(const bool isDash)
			{
				m_isDash = isDash;
			}
			/**
			 * @brief 泳ぎ中かどうかを設定
			 * @param isSwimming 泳ぎ中かどうか
			 */
			inline void SetIsSwimming(const bool isSwimming)
			{
				m_isSwimming = isSwimming;
			}

			/**
			 * @brief 物理挙動パラメータの設定
			 * @details プレイヤーとエネミーで個別の数値を設定するために使用します
			 */
			inline void SetPhysicsParams(float acceleration, float friction, float turnSpeed)
			{
				m_acceleration = acceleration;
				m_friction = friction;
				m_turnSpeed = turnSpeed;
			}


		public:
			/**
			 * @brief 移動処理
			 */
			void Move();


		protected:
			/**
			 * @brief 移動入力があるかどうか（スニーク/ダッシュ/スライドへの遷移判定に使用）
			 * @return 移動入力があるかどうか
			 */
			inline bool CanChangeMoveState() const
			{
				return fabsf(m_moveDirection.LengthSq()) > FLT_EPSILON;
			}
			/**
			 * @brief 走行ステートに切り替えられるかどうか
			 * @return 走行ステートに切り替えられるかどうか
			 */
			inline bool CanChangeRunState() const
			{
				return CanChangeMoveState() && m_isDash;
			}
			/**
			 * @brief 泳ぎステートに切り替えられるかどうか
			 * @return 泳ぎステートに切り替えられるかどうか
			 */
			inline bool CanChangeSwimState() const
			{
				return m_isSwimming || IsInWater();
			}
			/**
			 * @地面についているかどうか
			 * @return 地面についているかどうか
			 */
			inline bool IsOnGround() const
			{
				return m_ownerCharacter->GetCharacterController()->IsOnGround();
			}
			/**
			 * @brief 水の中にいるかどうか
			 * @details 現在の波面Y（CalcCurrentWaveY）を基準に判定する。
			 * @return 水の中にいるかどうか
			 */
			inline bool IsInWater() const
			{
				const float height = m_ownerActor->GetTransform().m_position.y;
				const float waveY = CalcCurrentWaveY();
				return height < waveY + SEA_SURFACE_THRESHOLD;
			}

			/**
			 * @brief 自身のXZ座標における現在の波面Yを取得する。
			 * @details g_renderingEngine->GetOcean() が nullptr の場合は SEA_LEVEL を返す。
			 * @return 波面Yオフセット
			 */
			float CalcCurrentWaveY() const;


		public:
			/**
			 * @brief ステートの変更先を取得する
			 * @return 変更先のステートポインタ
			 */
			virtual core::IState* GetChangeState() override;


		public:
			CharacterStateMachine(CharacterBase* ownerCharacter);
			virtual ~CharacterStateMachine() override = default;


		protected:
			/** キャラクターのオーナー */
			CharacterBase* m_ownerCharacter;


			/** 移動方向 */
			Vector3 m_moveDirection;
			/** 移動速度 */
			float m_moveSpeed;


			/** ダッシュしているかどうか */
			bool m_isDash;
			/** 泳ぎ中かどうか */
			bool m_isSwimming;
			/** 前フレームのY座標（水面を抜けた瞬間の判定に使用） */
			float m_prevPositionY;


		protected:
			/** 現在の移動速度ベクトル（慣性用） */
			Vector3 m_currentVelocity = Vector3::Zero;

			/** 加速度（目標速度への到達の早さ） */
			float m_acceleration = 10.0f;
			/** 摩擦係数（入力がないときの減速の早さ） */
			float m_friction = 5.0f;
			/** 旋回速度（Slerpによる回転の滑らかさ） */
			float m_turnSpeed = 8.0f;
		};

	}
}