#include "stdafx.h"
#include "CameraManager.h"


namespace app
{
	namespace camera
	{
		CameraManager* CameraManager::m_instance = nullptr;


		CameraManager::CameraManager()
		{
			//ばねカメラの初期化
			//m_springCamera = std::make_unique<SpringCamera>();
			//m_springCamera->Init(
			//    *g_camera3D,	//ばねカメラの処理を行うカメラを指定する
			//    1000.0f,		//カメラの移動速度の最大値
			//    true,			//カメラと地形とのあたり判定を取るかどうかのフラグ。trueだとあたり判定を行う
			//    5.0f			//カメラに設定される球体コリジョンの半径。第3引数がtrueの時に有効になる
			//);


			// カメラの初期化
			CameraSystem::Get().GetMainCamera().SetNear(0.01f);
			CameraSystem::Get().GetMainCamera().SetFar(5000.0f);

			Setup(&CameraSystem::Get().GetMainCamera());
		}


		CameraManager::~CameraManager()
		{}


		void CameraManager::Setup(nsK2EngineLow::Camera* engineCamera)
		{
			m_engineCamera = engineCamera;
		}


		void CameraManager::Register(const uint32_t nameHash, RefCameraController controller)
		{
			if (m_controllers.find(nameHash) != m_controllers.end()) {
				// すでに登録されている場合は無視
				return;
			}
			m_controllers[nameHash] = controller;
		}


		void CameraManager::Unregister(const uint32_t nameHash)
		{
			if (m_controllers.find(nameHash) == m_controllers.end()) {
				// 登録されていない場合は無視
				return;
			}
			m_controllers.erase(nameHash);
		}


		void CameraManager::SwitchCamera(const uint32_t nameHash, const float blendTime)
		{
			auto it = m_controllers.find(nameHash);
			if (it == m_controllers.end()) return;
			SwitchCamera(it->second, blendTime);
		}


		void CameraManager::SwitchCamera(RefCameraController controller, const float blendTime)
		{
#if defined(APP_DEBUG)
			m_prev = m_current;
#endif
			if (!m_current || blendTime <= 0.0f) {
				// 即時切り替え
				m_current = controller;
				m_current->OnEnter();
				m_next = nullptr;
				m_isBlending = false;
			}
			else {
				// ブレンド開始
				if (m_current != controller) {
					m_next = controller;
					m_next->OnEnter();

					// 現在のエンジンカメラの状態をブレンド開始地点として保存
					m_blendStartData.position = m_engineCamera->GetPosition();
					m_blendStartData.target = m_engineCamera->GetTarget();
					m_blendStartData.up = m_engineCamera->GetUp();
					m_blendStartData.fov = m_engineCamera->GetViewAngle();

					m_blendDuration = blendTime;
					m_blendTimer = 0.0f;
					m_isBlending = true;
				}
			}
		}


		void CameraManager::Update(const float deltaTime)
		{
			if (m_engineCamera == nullptr) {
				return;
			}

			CameraData applyData;

			// 各コントローラーのUpdate
			if (m_current) {
				m_current->Update();
			}
			if (m_next) {
				m_next->Update();
			}

			// 情報の決定
			if (m_isBlending && m_next) {
				m_blendTimer += deltaTime;
				const float t = m_blendTimer / m_blendDuration;
				// 保管終了
				if (t >= 1.0f) {
					m_isBlending = false;
					m_current = m_next;
					m_next = nullptr;
					applyData = m_current->GetCameraData();
				}
				else {
					// SmoothStepなどで滑らかにするとより良い
					// t = t * t * (3.0f - 2.0f * t); 
					applyData = CameraData::Lerp(t, m_blendStartData, m_next->GetCameraData());
				}
			}
			else if (m_current) {
				applyData = m_current->GetCameraData();
			}

			// 3. エンジンカメラへの反映
			m_engineCamera->SetPosition(applyData.position);
			m_engineCamera->SetTarget(applyData.target);
			m_engineCamera->SetUp(applyData.up);
			m_engineCamera->SetViewAngle(applyData.fov);
			m_engineCamera->SetNear(applyData.nearClip);
			m_engineCamera->SetFar(applyData.farClip);

			// 必要ならUpdateを呼ぶ
			// m_engineCamera->Update(); 
			//m_springCamera->Update();
		}
	}
}