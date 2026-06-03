/**
 * @file CameraSystem.h
 * @brief カメラシステム。メイン・サブカメラの保持と更新を行う
 * @author 竹林
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief カメラシステム
	 * @details メインカメラとサブカメラの実体を保持し、毎フレーム更新する。
	 *          SubCameraManager からサブカメラの生成・破棄を行う。
	 */
	class CameraSystem
	{
	public:
		/**
		 * @brief メインカメラを初期化する
		 */
		void Init();

		/**
		 * @brief 毎フレームの更新処理
		 * @details メイン・サブ両カメラを更新する
		 */
		void Update();

		/**
		 * @brief サブカメラを生成する
		 * @details SubCameraManager から呼び出す
		 */
		void CreateSubCamera();

		/**
		 * @brief サブカメラを破棄する
		 * @details SubCameraManager から呼び出す
		 */
		void DestroySubCamera();


	public:
		/**
		 * @brief メインカメラを取得する
		 * @return メインカメラの参照
		 */
		nsK2EngineLow::Camera& GetMainCamera()
		{
			return m_mainCamera;
		}

		/**
		 * @brief メインカメラを取得する（const版）
		 * @return メインカメラのconst参照
		 */
		const nsK2EngineLow::Camera& GetMainCamera() const
		{
			return m_mainCamera;
		}

		/**
		 * @brief サブカメラを取得する
		 * @return サブカメラのポインタ。未生成の場合はnullptr
		 */
		nsK2EngineLow::Camera* GetSubCamera()
		{
			return m_subCamera;
		}

		/**
		 * @brief サブカメラを取得する（const版）
		 * @return サブカメラのconstポインタ。未生成の場合はnullptr
		 */
		const nsK2EngineLow::Camera* GetSubCamera() const
		{
			return m_subCamera;
		}

		/**
		 * @brief サブカメラが有効かどうかを返す
		 * @return サブカメラが生成されていればtrue
		 */
		bool HasSubCamera() const
		{
			return m_subCamera != nullptr;
		}


	private:
		/** メインカメラ */
		nsK2EngineLow::Camera m_mainCamera;
		/** サブカメラ（未生成時はnullptr） */
		nsK2EngineLow::Camera* m_subCamera = nullptr;


		//============================================//
		// シングルトン関連
		//============================================//

	private:
		CameraSystem() = default;
		~CameraSystem() = default;

		static CameraSystem* m_instance;


	public:
		/** @brief シングルトンインスタンスを生成する */
		static void CreateInstance()
		{
			if (m_instance == nullptr) {
				m_instance = new CameraSystem();
			}
		}
		/** @brief シングルトンインスタンスを取得する */
		static CameraSystem& Get()
		{
			return *m_instance;
		}
		/** @brief シングルトンインスタンスを破棄する */
		static void DestroyInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}
	};
}
