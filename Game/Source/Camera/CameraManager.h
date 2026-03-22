/**
 * CameraManager.h
 * カメラ管理
 */
#pragma once
#include "CameraCommon.h"


namespace app
{
	namespace camera
	{
		class CameraManager
		{
		private:
			std::map<uint32_t, std::shared_ptr<ICameraController>> m_controllers;
			std::shared_ptr<ICameraController> m_current;
			std::shared_ptr<ICameraController> m_next;
#if defined(APP_DEBUG)
			std::shared_ptr<ICameraController> m_prev;
#endif

			//std::unique_ptr<SpringCamera> m_springCamera;

			/** ブレンド用 */
			bool m_isBlending = false;
			float m_blendDuration = 0.0f;
			float m_blendTimer = 0.0f;
			CameraData m_blendStartData;

			/** 描画で使用されるカメラ */
			nsK2EngineLow::Camera* m_engineCamera = nullptr;


		private:
			CameraManager();
			~CameraManager();


		public:
			/**
			 * 初期化
			 * エンジン側の実体カメラを設定
			 */
			void Setup(nsK2EngineLow::Camera* engineCamera);

			/**
			 * 更新処理
			 * ブレンド計算とエンジンカメラへの反映
			 */
			void Update(const float deltaTime);

			/** コントローラーの登録・解除 */
			void Register(const uint32_t nameHash, RefCameraController controller);
			void Unregister(const uint32_t nameHash);

			/**
			 * カメラ切り替え
			 * NOTE: blendTime秒かけて遷移し0なら即時。
			 */
			void SwitchCamera(const uint32_t nameHash, const float blendTime = 0.0f);
			void SwitchCamera(RefCameraController controller, const float blendTime = 0.0f);
#if defined(APP_DEBUG)
			/** デバッグ用: 前のカメラに戻す */
			void SwitchPrevCamera(const float blendTime = 0.0f)
			{
				if (m_prev) {
					SwitchCamera(m_prev, blendTime);
				}
			}
#endif // APP_DEBUG

			/** 現在のカメラデータを取得 */
			const CameraData& GetCurrentCameraData() const
			{
				if (m_current) {
					return m_current->GetCameraData();
				}
				static CameraData defaultData;
				return defaultData;
			}


			/**
			 * @brief 指定したIDのコントローラーを取得する
			 * @tparam T 取得したいコントローラーの型
			 * @param nameHash コントローラーのID（例: WinCamera::ID()）
			 * @return コントローラーのshared_ptr。見つからない場合はnullptr
			 */
			template <typename T>
			std::shared_ptr<T> GetController(const uint32_t nameHash)
			{
				auto it = m_controllers.find(nameHash);
				if (it != m_controllers.end()) {
					// ICameraController から派生クラス（VictoryCameraなど）へキャスト
					return std::static_pointer_cast<T>(it->second);
				}
				return nullptr;
			}




			/**
			 * シングルトン関連
			 */
		private:
			static CameraManager* m_instance;


		public:
			static void CreateInstance()
			{
				if (m_instance == nullptr) {
					m_instance = new CameraManager();
				}
			}
			static CameraManager& Get() { return *m_instance; }
			static void DestroyInstance()
			{
				delete m_instance;
				m_instance = nullptr;
			}
		};
	}
}