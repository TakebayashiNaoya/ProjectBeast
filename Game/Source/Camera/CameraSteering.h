/**
 * @file CameraSteering.h
 * @brief カメラ操縦処理群
 */
#pragma once
#include "CameraCommon.h"


namespace app
{
	namespace actor
	{
		class CharacterBase;
	}

	namespace camera
	{
		/**
		 * @brief ターゲットキャラクターを中心に、右スティックで回転させるタイプのカメラコントローラー
		 */
		class CameraSteering : public Noncopyable
		{
		public:
			struct Config
			{
				float distance = 200.0f;
				float height = 100.0f;
				float rotationSpeedX = 2.5f;
				float rotationSpeedY = 2.5f;
			};


		private:
			Config m_config;
			app::actor::CharacterBase* m_targetCharacter = nullptr;
			Vector3 m_toVector = Vector3::Zero;

			/** バネ追従中のカメラ位置（理想位置へ時定数付きで追いつく） */
			Vector3 m_smoothedPosition = Vector3::Zero;
			/** バネ追従の初期化済みフラグ（初回とワープ時はスナップする） */
			bool m_isPositionInitialized = false;
			/** 前フレームのターゲット座標（速度推定用） */
			Vector3 m_prevTargetPos = Vector3::Zero;
			/** 平滑化済みのターゲット速度（ユニット/秒） */
			float m_smoothedSpeed = 0.0f;
			/** 平滑化済みの先読みオフセット（移動方向へ注視点をずらす量） */
			Vector3 m_lookAheadOffset = Vector3::Zero;
			/** 現在の視野角（ラジアン。速度に応じて基準値〜最大値を行き来する） */
			float m_currentFov = 0.0f;


		public:
			/**
			 * @brief カメラの更新処理
			 * @param data カメラの姿勢データ
			 * @param deltaTime 前フレームからの経過時間
			 */
			void Update(CameraData& data, const float deltaTime);
			/**
			 * @brief カメラの設定
			 * @param config カメラの設定
			 */
			void SetConfig(const Config& config)
			{
				m_config = config;

				m_toVector.z = -m_config.distance;
				m_toVector.y = m_config.height;
			}
			/**
			 * @brief ターゲットキャラクターの設定
			 * @param character ターゲットキャラクター
			 */
			void SetTargetCharacter(app::actor::CharacterBase* character) { m_targetCharacter = character; }
		};
	}
}