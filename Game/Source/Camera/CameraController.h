/**
 * CameraController.h
 * カメラコントローラー群
 */
#pragma once
#include "CameraCommon.h"


namespace app
{
	namespace camera
	{
		/**
		 * ゲーム内で外部から状態をセットして使うタイプのカメラコントローラー
		 */
		class GameCamera : public ICameraController
		{
			appCameraController(GameCamera);


		private:
			/** 演出を乗せたあとの最終的なカメラ状態（描画に使われる） */
			CameraData m_data;
			/** SetState() で受け取った素のカメラ状態。演出はここから毎フレーム作り直す */
			CameraData m_baseData;

			float m_shakeTimer = 0.0f;      /** 画面揺れの残り時間（秒） */
			float m_shakeDuration = 0.0f;   /** 画面揺れの全体時間（秒） */
			float m_shakeStrength = 0.0f;   /** 画面揺れの最大振幅（ワールド単位） */
			float m_shakeTime = 0.0f;       /** 画面揺れの経過時間（ノイズの位相に使う、秒） */

			float m_punchTimer = 0.0f;      /** パンチイン（一瞬寄る）の残り時間（秒） */
			float m_punchDuration = 0.0f;   /** パンチインの全体時間（秒） */
			float m_punchAmount = 0.0f;     /** パンチインで詰める距離の割合（0.15 = 15%） */


		public:
			/**
			 * 外部から状態をセットする
			 * NOTE: BattleManagerなどが呼ぶ
			 */
			void SetState(const CameraData& data)
			{
				m_baseData = data;
				m_data = data;
			}

			/**
			 * @brief 画面揺れを開始する
			 * @details 咆哮・密集陣の弾き反撃・イグルー破壊などの衝撃演出用。
			 *          振幅は残り時間に比例して減衰する。
			 * @param strength 最大振幅（ワールド単位。8〜15程度が目安）
			 * @param duration 揺れの長さ（秒）
			 */
			void StartShake(const float strength, const float duration)
			{
				m_shakeStrength = strength;
				m_shakeDuration = duration;
				m_shakeTimer = duration;
			}

			/**
			 * @brief パンチイン（注視点へ一瞬寄って戻る）を開始する
			 * @details ウルト発動などの「タメ」の瞬間用。sinカーブで寄って戻る。
			 * @param amount   距離を詰める割合（0.15 = 15%）
			 * @param duration 演出の長さ（秒）
			 */
			void StartPunchIn(const float amount, const float duration)
			{
				m_punchAmount = amount;
				m_punchDuration = duration;
				m_punchTimer = duration;
			}

			void Update() override;

			const CameraData& GetCameraData() const override { return m_data; }
		};




		/**
		 * ステージ紹介動画の撮影用カメラコントローラー
		 * NOTE: 環境変数 BEAST_SHOWCASE が立っているときだけ InGameSceneBase が登録する。
		 *       ステージ中心を見ながら外周をゆっくり一周するフライスルー。
		 *       ステージ選択画面の動画（Assets/Video/*Stage.mp4）を撮り直すための専用カメラ
		 */
		class ShowcaseCamera : public ICameraController
		{
			appCameraController(ShowcaseCamera);


		private:
			CameraData m_data;
			float m_time = 0.0f;  /** 撮影開始からの経過時間（秒） */


		public:
			void Update() override;

			const CameraData& GetCameraData() const override { return m_data; }
		};




		/**
		 * リプレイ再生用カメラコントローラー
		 * NOTE: ReplaySceneが呼ぶ
		 */
		class ReplayCamera : public ICameraController
		{
			appCameraController(ReplayCamera);


		private:
			CameraData m_data;


		public:
			/**
			 * 外部から状態をセットする
			 * NOTE: ReplaySceneが再生中のtickから補間した値を毎フレームセットする
			 */
			void SetState(const CameraData& data)
			{
				m_data = data;
			}

			void Update() override
			{}

			const CameraData& GetCameraData() const override { return m_data; }
		};




#if defined(APP_DEBUG)
		/**
		 * デバッグ用カメラコントローラー
		 */
		class DebugCamera : public ICameraController
		{
			appCameraController(DebugCamera);


		private:
			CameraData m_cameraData;


		public:
			void OnEnter() override;

			void Update() override;

			const CameraData& GetCameraData() const override { return m_cameraData; }
		};
#endif // APP_DEBUG
	}
}