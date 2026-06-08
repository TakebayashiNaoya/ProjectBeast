/**
 * @file BeastEngine.cpp
 * @brief BeastEngineクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "BeastEngine.h"
#include "Graphics/Camera/SubCameraManager.h"


namespace nsBeastEngine
{
	/** 静的メンバ変数の実体を定義 */
	BeastEngine* BeastEngine::m_instance = nullptr;
	BeastEngine* g_beastEngine = nullptr;
	SceneLight* g_sceneLight = nullptr;
	RenderingEngine* g_renderingEngine = nullptr;


	BeastEngine::~BeastEngine()
	{
		SubCameraManager::DestroyInstance();
		CameraSystem::DestroyInstance();
		g_renderingEngine = nullptr;
		g_engine = nullptr;
	}


	void BeastEngine::Init(const InitData& initData)
	{
		g_beastEngine = this;
		g_engine = &m_k2EngineLow;
		g_renderingEngine = &m_renderingEngine;

		m_k2EngineLow.Init(
			initData.hwnd,
			initData.frameBufferWidth,
			initData.frameBufferHeight
		);

		// サブカメラマネージャーの初期化
		SubCameraManager::CreateInstance();

		// カメラシステムの初期化（メインカメラの生成）
		CameraSystem::CreateInstance();
		CameraSystem::Get().Init();

		// メインカメラの初期位置を設定
		CameraSystem::Get().GetMainCamera().SetPosition({ 0.0f, 100.0f, -200.0f }); /** 手前・上に配置 */
		CameraSystem::Get().GetMainCamera().SetTarget({ 0.0f, 50.0f, 0.0f });       /** 原点より少し上を見る */

		m_renderingEngine.Init();

		/** モデルリソースを登録 */
		ResourceManager::GetInstance().Register<TkmResource>(std::make_shared<TkmLoader>());
		/** アニメーションリソースを登録 */
		ResourceManager::GetInstance().Register<TkaResource>(std::make_shared<TkaLoader>());
		/** スケルトンリソースを登録 */
		ResourceManager::GetInstance().Register<TksResource>(std::make_shared<TksLoader>());
		/** リソースマネージャーを起動 */
		ResourceManager::GetInstance().Start();
		/** トゥーンシェーダーのグローバル設定を有効化 */
		//ModelRender::SetToonGlobalEnabled(true);
	}


	void BeastEngine::BeginExecute()
	{
		// フレーム開始
		g_engine->BeginFrame();

		// k2EngineLowの更新処理
		g_engine->ExecuteUpdate();

		// カメラの更新
		SubCameraManager::Get().Update();
		CameraSystem::Get().Update();

		// レンダリングエンジンの更新
		m_renderingEngine.Update();

		// k2EngineLowの描画処理
		g_engine->ExecuteRender();

#ifdef DEBUG
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DebubDrawWorld(g_graphicsEngine->GetRenderContext());
#endif // DEBUG
	}


	void BeastEngine::EndExecute()
	{
		// 描画処理
		m_renderingEngine.Execute(g_graphicsEngine->GetRenderContext());

		// 当たり判定描画
		g_engine->DebubDrawWorld();

		// フレーム終了
		g_engine->EndFrame();
	}


	void BeastEngine::CreateInstance(const InitData& initData)
	{
		if (m_instance == nullptr) {
			m_instance = new BeastEngine();
			m_instance->Init(initData);
		}
	}


	void BeastEngine::DeleteInstance()
	{
		delete m_instance;
		m_instance = nullptr;
	}
}