/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシン
 */
#pragma once
#include "Actor.h"
#include "Source/Core/StateMachineBase.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief アクターのステートマシン
		 */
		class ActorStateMachine : public core::StateMachineBase
		{
		public:
			/** アニメーション切り替え時の標準ブレンド時間（秒）。
			 *  0だと前のポーズから瞬間的に切り替わってカクつくため、
			 *  全キャラ共通で短いクロスフェードを挟む */
			static constexpr float ANIMATION_INTERPOLATE_TIME = 0.15f;

			/**
			 * @brief アニメーションの再生
			 * @tparam TEnum アニメーションIDの型
			 * @param animationID アニメーションID
			 * @param interpolateTime ブレンド時間（秒）。省略時は標準ブレンド
			 */
			template<typename TEnum>
			inline void PlayAnimation(TEnum animationID, float interpolateTime = ANIMATION_INTERPOLATE_TIME)
			{
				m_ownerActor->GetModelRender()->PlayAnimation(
					static_cast<uint8_t>(animationID), interpolateTime);
			}
			/**
			 * @brief アニメーションの再生速度の設定
			 * @param speed 再生速度（1.0f=通常速度）
			 */
			inline void SetAnimationSpeed(float speed)
			{
				m_ownerActor->SetAnimationSpeed(speed);
			}
			/**
			 * @brief アニメーションが再生中か
			 * @return 再生中ならtrue
			 */
			inline bool IsPlayingAnimation() const
			{
				return m_ownerActor->GetModelRender()->IsPlayingAnimation();
			}
			/**
			 * @brief トランスフォームの取得
			 * @return トランスフォームの参照
			 */
			inline const core::Transform& GetTransform() const
			{
				return m_transform;
			}
			/**
			 * @brief 座標の設定
			 * @param position 座標
			 */
			inline void SetPosition(const Vector3& position)
			{
				m_transform.m_position = position;
			}
			/**
			 * @brief 回転の設定
			 * @param rotation 回転
			 */
			inline void SetRotation(const Quaternion& rotation)
			{
				m_transform.m_rotation = rotation;
			}
			/**
			 * @brief 拡大率の設定
			 * @param scale 拡大率
			 */
			inline void SetScale(const Vector3& scale)
			{
				m_transform.m_scale = scale;
			}

			inline void SetActive(const bool isActive)
			{
				m_ownerActor->SetActive(isActive);
			}


		public:
			/**
			 * @brief ステートの変更先を取得する
			 * @return 変更先のステートポインタ
			 */
			virtual core::IState* GetChangeState() override;


		public:
			ActorStateMachine(Actor* ownerActor);
			virtual ~ActorStateMachine() override = default;


		protected:
			/** アクターのオーナー */
			Actor* m_ownerActor;
			/** ステートマシンが持つトランスフォーム */
			core::Transform m_transform;

		};

	}
}