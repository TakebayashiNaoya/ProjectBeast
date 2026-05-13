/**
 * @file Actor.h
 * @brief 見た目が存在するオブジェクトの基底クラス
 * @author 藤谷
 */
#pragma once
#include "ActorStatus.h"
#include "IObject.h"
#include "Source/Core/Transform.h"
#include "Source/Util/Curve.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			constexpr float MIN_ANIMATION_SPEED = 0.1f; // アニメーションの再生速度の最小値
			constexpr float MAX_ANIMATION_SPEED = 3.0f; // アニメーションの再生速度の最大値
		}


		/**
		 * @brief 見た目が存在するオブジェクトの基底クラス
		 */
		class Actor : public IObject
		{
		public:
			/**
			 * @brief トランスフォームを取得
			 * @return トランスフォームの参照
			 */
			inline const core::Transform& GetTransform() const { return m_transform; }
			/**
			 * @brief モデルレンダーを取得
			 * @return モデルレンダーのポインタ
			 */
			inline ModelRender* GetModelRender() { return &m_modelRender; }
			/**
			 * @brief ステータスを取得
			 * @tparam TStatus ステータスの型
			 * @return ステータスのポインタ
			 */
			template<typename TStatus>
			inline TStatus* GetStatus() const { return dynamic_cast<TStatus*>(m_status.get()); }
			/**
			 * @brief 座標を設定
			 * @param position 座標
			 */
			void SetPosition(const Vector3 position) { m_transform.m_position = position; }
			/**
			 * @brief 回転を設定
			 * @param rotation 回転
			 */
			void SetRotation(const Quaternion rotation) { m_transform.m_rotation = rotation; }
			/**
			 * @brief 拡大率を設定
			 * @param scale 拡大率
			 */
			void SetScale(const Vector3 scale) { m_transform.m_scale = scale; }
			/**
			 * @brief アニメーション再生速度の設定
			 * @param speed 再生速度（1.0f=通常速度）
			 */
			void SetAnimationSpeed(float speed)
			{
				speed = std::clamp(speed, MIN_ANIMATION_SPEED, MAX_ANIMATION_SPEED);
				m_modelRender.SetAnimationSpeed(speed);
			}


		public:
			Actor() = default;
			virtual ~Actor() = default;


		protected:
			/** 初期化処理 */
			virtual void Start()override = 0;
			/** 更新処理 */
			virtual void Update()override = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override = 0;


		protected:
			/** トランスフォーム */
			core::Transform m_transform;
			/** モデルレンダー */
			ModelRender m_modelRender;
			/** ステータス */
			std::unique_ptr<ActorStatus> m_status;
		};
	}
}

