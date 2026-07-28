#include "k2EngineLowPreCompile.h"
#include "k2EngineLow.h"
#include "graphics/Texture.h"

namespace nsK2EngineLow {
	K2EngineLow* g_engine = nullptr;
	GameTime* g_gameTime = nullptr;

	K2EngineLow::~K2EngineLow()
	{
		// グローバルなアクセスポイントにnullptrを代入。
		g_graphicsEngine = nullptr;
		g_gameTime = nullptr;

		delete m_graphicsEngine;

		//ゲームオブジェクトマネージャーを削除。
		GameObjectManager::DeleteInstance();
		//PhysicsWorld::DeleteInstance();
		EffectEngine::DeleteInstance();

		delete g_soundEngine;
	}
	void K2EngineLow::Init(HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
	{
		if (hwnd) {
			//グラフィックエンジンの初期化。
			m_graphicsEngine = new GraphicsEngine();
			m_graphicsEngine->Init(hwnd, frameBufferWidth, frameBufferHeight);
		}
		g_gameTime = &m_gameTime;
		//ゲームパッドの初期化。
		for (int i = 0; i < GamePad::CONNECT_PAD_MAX; i++) {
			g_pad[i] = &m_pad[i];
		}

		GameObjectManager::CreateInstance();
		//PhysicsWorld::CreateInstance();
		g_soundEngine = new SoundEngine();
		if (m_graphicsEngine) {
			//エフェクトエンジンの初期化。
			EffectEngine::CreateInstance();
		}
#ifdef K2_DEBUG
		if (m_graphicsEngine) {
			m_fpsFont = std::make_unique<Font>();
			m_fpsFontShadow = std::make_unique<Font>();
		}
#endif
		g_engine = this;

		// ゲームループ全体（application->Update() を含む）を計測区間に収めるため、
		// 最初の一回だけここで計測を開始する。以降は EndMeasurement() が
		// 計測終了と同時に次の区間を継続して開始する。
		m_gameTime.BeginMeasurement();
	}
	void K2EngineLow::BeginFrame()
	{
		m_fpsLimitter.BeginFrame();
		m_graphicsEngine->BeginRender();
		EffectEngine::GetInstance()->BeginFrame();
		for (auto& pad : m_pad) {
			pad.BeginFrame();
		}

	}
	void K2EngineLow::EndFrame()
	{
#ifdef K2_DEBUG
		m_fpsFont->Begin(g_graphicsEngine->GetRenderContext());
		float time = g_gameTime->GetFrameDeltaTime();
		wchar_t text[256];
		swprintf(text, L"FPS = %0.2f", 1.0f / time);
		m_fpsFontShadow->Draw(text, { UI_SPACE_WIDTH * -0.48f + 3.0f , UI_SPACE_HEIGHT * 0.48f - 3.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 1.0f, { 0.0f, 1.0f });
		m_fpsFont->Draw(text, { UI_SPACE_WIDTH * -0.48f, UI_SPACE_HEIGHT * 0.48f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f, 1.0f, { 0.0f, 1.0f });
		m_fpsFont->End(g_graphicsEngine->GetRenderContext());
#endif 
		m_graphicsEngine->EndRender();
#ifdef USE_FPS_LIMITTER
		m_fpsLimitter.Wait();
#endif
		m_gameTime.EndMeasurement();

	}

	void K2EngineLow::ExecuteUpdate()
	{
		// GameObjectManagerとEffectEngineの更新は上位層側で行う。
		// ここでは常に動かし続けたいパッド・サウンドのみ更新する。
		for (auto& pad : m_pad) {
			pad.Update();
		}
		g_soundEngine->Update();
	}
	/// <summary>
	/// 描画処理を実行。
	/// </summary>
	void K2EngineLow::ExecuteRender()
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();
		// ゲームオブジェクトマネージャーの描画処理を実行。
		GameObjectManager::GetInstance()->ExecuteRender(renderContext);

	}

	/// <summary>
	/// 当たり判定描画処理を実行。
	/// </summary>
	void K2EngineLow::DebubDrawWorld()
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();
		//当たり判定描画処理を実行。
		//PhysicsWorld::GetInstance()->DebubDrawWorld(renderContext);
	}
}