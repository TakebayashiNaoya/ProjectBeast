/**
 * @file CameraSystem.h
 * @brief カメラシステム。メイン・サブカメラの保持と更新を行う
 */
#pragma once


namespace nsBeastEngine
{


	/**
	 * @brief カメラシステム
	 * @details メインカメラとサブカメラの実体を保持し、毎フレーム更新する。
	 *          メインカメラは g_camera3D をラップして提供する。
	 *          g_camera3D の Update は GraphicsEngine::BeginRender() が行うため、
	 *          CameraSystem::Update() ではサブカメラのみ更新する。
	 *          サブカメラの生成・破棄は SubCameraManager から呼び出す。
	 */
	class CameraSystem
	{
	public:
		/**
		 * @brief 初期化する
		 * @details g_camera3D をメインカメラとして設定する
		 */
		void Init();

		/**
		 * @brief 毎フレームの更新処理
		 * @details サブカメラのみ更新する。メインカメラは GraphicsEngine が更新する
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
			return *m_mainCamera;
		}

		/**
		 * @brief メインカメラを取得する（const版）
		 * @return メインカメラのconst参照
		 */
		const nsK2EngineLow::Camera& GetMainCamera() const
		{
			return *m_mainCamera;
		}

		/**
		 * @brief 現在描画中のアクティブカメラを取得する
		 * @details RenderingEngineがExecuteViewPass前にSetActiveCameraで設定する。
		 *          描画パス外では m_mainCamera と同じ。
		 * @return アクティブカメラのポインタ
		 */
		nsK2EngineLow::Camera* GetActiveCamera()
		{
			return m_activeCamera;
		}

		/**
		 * @brief アクティブカメラを設定する
		 * @details RenderingEngine::ExecuteViewPass からのみ呼ぶこと
		 * @param camera 設定するカメラ
		 */
		void SetActiveCamera(nsK2EngineLow::Camera* camera)
		{
			m_activeCamera = camera;
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
		/** メインカメラ（g_camera3D をラップ。実体は k2EngineLow が所有） */
		nsK2EngineLow::Camera* m_mainCamera = nullptr;
		/** サブカメラ */
		nsK2EngineLow::Camera* m_subCamera = nullptr;
		/** 現在描画中のカメラ（ExecuteViewPass前にセットされる） */
		nsK2EngineLow::Camera* m_activeCamera = nullptr;


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
			if (m_instance == nullptr)
			{
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


} // namespace nsBeastEngine