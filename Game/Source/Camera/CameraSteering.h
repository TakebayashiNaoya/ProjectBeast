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

			/** ターゲットからの距離に掛ける倍率（陣形レベルで引く） */
			float m_zoomFactor = 1.0f;
			/** 倍率の補間の始まりの値 */
			float m_zoomFrom = 1.0f;
			/** 倍率の補間の行き先 */
			float m_zoomTo = 1.0f;
			/** 倍率の補間を始めてからの時間（秒） */
			float m_zoomElapsed = 0.0f;
			/** 倍率の補間にかける時間（秒） */
			float m_zoomDuration = 0.0f;
			/** 視野角を一瞬広げる量（度） */
			float m_fovKickDeg = 0.0f;
			/** 視野角を広げきるまでの時間（秒） */
			float m_fovKickAttackTime = 0.0f;
			/** 視野角が元へ戻るまでの時間（秒） */
			float m_fovKickReturnTime = 0.0f;
			/** 視野角を広げてからの経過時間（秒） */
			float m_fovKickElapsed = 0.0f;


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
			/**
			 * @brief ターゲットからの距離の倍率をtargetへ動かす（動き出しが速く、最後に減速する）
			 * @param target   行き先の倍率（1で元の距離）
			 * @param duration 動かす時間（秒。0以下ならすぐ切り替える）
			 */
			void StartZoom(const float target, const float duration);
			/**
			 * @brief 視野角を広げてから元へ戻す（カメラを引いたことを見せる）
			 * @param addDeg     広げる角度（度）
			 * @param attackTime 広げきるまでの時間（秒）
			 * @param returnTime 元へ戻るまでの時間（秒）
			 */
			void StartFovKick(const float addDeg, const float attackTime, const float returnTime);
		};
	}
}