/**
 * @file CharacterController.h
 * @brief キャラクターコントローラー
 * @author 竹林尚哉
 */
#pragma once
#include "CapsuleCollider.h"
#include "RigidBody.h"


namespace nsBeastEngine
{
	namespace nsCollision
	{
		/**
		 * @brief キャラクターコントローラー
		 */
		class CharacterController : public Noncopyable
		{
		public:
			/**
			 * @brief	テレポートをリクエストする
			 * @details	これを呼んだ次の Execute() で、衝突判定を行わずに targetPosition へ移動する
			 */
			inline void RequestTeleport() { m_isRequestTeleport = true; }

			/**
			 * @brief 重力加速度を設定
			 * @param gravity 重力加速度
			 */
			inline void SetGravity(const float gravity) { m_gravity = gravity; }

			/**
			 * @brief 座標を取得
			 * @return 座標
			 */
			inline const Vector3& GetPosition() const { return m_position; }
			/**
			 * @brief 前回の座標を取得
			 * @return 前回の座標
			 */
			inline const Vector3& GetPrevPosition() const { return m_prevPosition; }

			/**
			 * @brief 座標を設定
			 * @param position 座標
			 */
			inline void SetPosition(const Vector3& position) { m_position = position; }

			/**
			 * @brief 垂直方向の速度を取得
			 * @return 垂直方向の速度
			 */
			inline float GetVerticalVelocity() const { return m_verticalVelocity; }

			/**
			 * @brief ジャンプ中フラグを取得
			 * @return ジャンプ中フラグ
			 */
			inline bool IsJump() const { return m_isJump; }

			/**
			 * @brief 地面に接地しているフラグを取得
			 * @return 地面に接地しているフラグ
			 */
			inline bool IsOnGround() const { return m_isOnGround; }

			/**
			 * @brief コライダーを取得
			 * @return コライダー
			 */
			inline CapsuleCollider* GetCollider() { return &m_collider; }
			/**
			 * @brief リジッドボディを取得
			 * @return リジッドボディ
			 */
			inline RigidBody* GetRigidBody() { return &m_rigidBody; }


		public:
			CharacterController();
			~CharacterController();

			/**
			 * @brief 初期化
			 * @param radius 半径
			 * @param height 高さ
			 * @param position 位置
			 */
			void Init(float radius, float height, const Vector3& position);

			/**
			 * @brief	実行
			 * @details テレポートリクエストがある場合は衝突判定を行わずに移動する
			 * @param targetPosition	移動先の座標
			 * @param deltaTime			経過時間
			 */
			const Vector3& Execute(const Vector3& targetPosition, float deltaTime);

			/**
			 * @brief ジャンプする
			 * @param jumpPower ジャンプパワー
			 */
			void Jump(float jumpPower);

			/**
			 * @brief リジッドボディを削除
			 */
			void RemoveRigidBoby();

			/**
			 * @brief バウンスする
			 * @param power バウンスパワー
			 */
			void Bounce(const float power);


		private:
			/** キャラクター用なのでカプセルとしておく */
			CapsuleCollider	m_collider;
			/** 物理空間の処理に必要 */
			RigidBody m_rigidBody;

			/** 座標 */
			Vector3 m_position;
			/** 前回の座標 */
			Vector3 m_prevPosition;

			/** 垂直方向の速度 */
			float m_verticalVelocity;
			/** 重力加速度 */
			float m_gravity;
			/** 半径 */
			float m_radius;
			/** 高さ */
			float m_height;

			/** 初期化済みフラグ */
			bool m_isInited;
			/** ジャンプ中フラグ */
			bool m_isJump;
			/** 地面に接地しているフラグ */
			bool m_isOnGround;
			/** テレポートリクエストフラグ */
			bool m_isRequestTeleport;
		};
	}
}