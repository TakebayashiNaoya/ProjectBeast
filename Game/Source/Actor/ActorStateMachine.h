/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシン
 * @author 藤谷
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
			/**
			 * @brief アニメーションの再生
			 * @tparam TEnum アニメーションIDの型
			 * @param animationID アニメーションID
			 */
			template<typename TEnum>
			inline void PlayAnimation(TEnum animationID)
			{
				m_ownerActor->GetModelRender()->PlayAnimation(static_cast<uint8_t>(animationID));
			}
			/**
			 * @brief トランスフォームの取得
			 * @return トランスフォームの参照
			 */
			inline const core::Transform& GetTransform() const
			{
				return m_ownerActor->GetTransform();
			}
			/**
			 * @brief 座標の設定
			 * @param position 座標
			 */
			inline void SetPosition(const Vector3& position)
			{
				m_ownerActor->SetPosition(position);
			}
			/**
			 * @brief 回転の設定
			 * @param rotation 回転
			 */
			inline void SetRotation(const Quaternion& rotation)
			{
				m_ownerActor->SetRotation(rotation);
			}
			/**
			 * @brief 拡大率の設定
			 * @param scale 拡大率
			 */
			inline void SetScale(const Vector3& scale)
			{
				m_ownerActor->SetScale(scale);
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


		};

	}
}