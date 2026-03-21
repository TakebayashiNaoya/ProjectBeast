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
            CameraData m_data;


        public:
            /**
             * 外部から状態をセットする
             * NOTE: BattleManagerなどが呼ぶ
             */
            void SetState(const CameraData& data)
            {
                m_data = data;
            }

            void Update() override
            {
                // 基本何もしない。
                // 必要ならここで画面揺れ(Shake)のオフセットを加算する処理などを入れても良い
            }

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