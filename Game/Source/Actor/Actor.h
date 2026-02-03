#pragma once
#include "Source/Core/Transform.h"





namespace app
{
	namespace actor
	{
		// 前方宣言
		class ActorStatus;

		class Actor : public IGameObject
		{
		protected:
			/** トランスフォーム */
			core::Transform m_transform;
			/** モデルレンダー */
			ModelRender m_modelRender;
			/** ステータス */
			std::unique_ptr<ActorStatus> m_status;


		public:
			/** トランスフォームを取得 */
			inline const core::Transform& GetTransform() const { return m_transform; }


			/** モデルレンダーを取得 */
			inline ModelRender* GetModelRender() { return &m_modelRender; }


			/** ステータスを取得する */
			template<typename TStatus>
			inline const TStatus* GetStatus() { return dynamic_cast<TStatus>(m_status.get()); }


			/** 座標を設定 */
			void SetPosition(const Vector3& position) { m_transform.m_position = position; }


			/** 回転を設定 */
			void SetRotation(const Quaternion& rotation) { m_transform.m_rotation = rotation; }


			/** 拡縮を設定 */
			void SetScale(const Vector3& scale) { m_transform.m_scale = scale; }


		public:
			Actor() = default;
			virtual ~Actor() = default;


		public:
			virtual bool Start()override = 0;
			virtual void Update()override = 0;
			virtual void Render(RenderContext& rc)override = 0;
		};
	}
}

