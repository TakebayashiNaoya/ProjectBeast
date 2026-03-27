/**
 * @file CameraSteering.h
 * @brief カメラ操縦処理群
 * @author 藤谷
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
				//float distance = 50.0f;
				//float height = 10.0f;
				//float rotationSpeedX = 1.0f;
				//float rotationSpeedY = 1.0f;
        
				float distance = 200.0f;
				float height = 100.0f;
				float rotationSpeedX = 0.1f;
				float rotationSpeedY = 0.1f;
			};


		private:
			Config m_config;
			app::actor::CharacterBase* m_targetCharacter = nullptr;
			Vector3 m_toVector = Vector3::Zero;


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

				m_toVector.z = m_config.distance;
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