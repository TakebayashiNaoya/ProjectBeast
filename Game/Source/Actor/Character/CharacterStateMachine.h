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
			/** 海面の高さ */
			constexpr float SEA_LEVEL = 0.0f;
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


		public:
			/**
			 * @brief 移動処理
			 */
			void Move();


		protected:
			/**
			 * @brief 歩行ステートに切り替えられるかどうか
			 * @return 歩行ステートに切り替えられるかどうか
			 */
			inline bool CanChangeWalkState() const
			{
				return fabsf(m_moveSpeed) > FLT_EPSILON && fabsf(m_moveDirection.LengthSq()) > FLT_EPSILON;
			}
			/**
			 * @brief 走行ステートに切り替えられるかどうか
			 * @return 走行ステートに切り替えられるかどうか
			 */
			inline bool CanChangeRunState() const
			{
				return CanChangeWalkState() && m_isDash;
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
			 * @return 水の中にいるかどうか
			 */
			inline bool IsInWater() const
			{
				const float height = m_ownerActor->GetTransform().m_position.y;
				return height < SEA_LEVEL;
			}


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
		};

	}
}