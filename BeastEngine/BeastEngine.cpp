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

	namespace
	{
		constexpr float k_imguiFontSize = 18.0f;  ///< ImGuiフォントサイズ（px）
		constexpr int   k_imguiFrameBufferCount = 2;      ///< スワップチェーンのフレームバッファ数
		constexpr UINT  k_imguiSrvDescriptorCount = 1;      ///< ImGui専用SRVヒープのディスクリプタ数
	}


	BeastEngine::~BeastEngine()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

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

		 // ========== ImGui 初期化 ==========
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		// 日本語フォント（メイリオ）を読み込む設定を追加
		io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\meiryo.ttc", k_imguiFontSize, NULL, io.Fonts->GetGlyphRangesJapanese());

		// Win32バックエンド初期化
		ImGui_ImplWin32_Init(initData.hwnd);

		// SRVヒープの作成（ImGui専用）
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		g_graphicsEngine->GetD3DDevice()->CreateDescriptorHeap(
			&heapDesc, IID_PPV_ARGS(&m_imguiSrvHeap)
		);

		// DX12バックエンド初期化
		ImGui_ImplDX12_Init(
			g_graphicsEngine->GetD3DDevice(),
			k_imguiFrameBufferCount,                              // フレームバッファ数
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_imguiSrvHeap.Get(),
			m_imguiSrvHeap->GetCPUDescriptorHandleForHeapStart(),
			m_imguiSrvHeap->GetGPUDescriptorHandleForHeapStart()
		);

		// 初回フレームでのクラッシュを防ぐため、フォントアトラスを事前に構築する
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	}


	void BeastEngine::BeginExecute()
	{
		// フレーム開始
		g_engine->BeginFrame();

		// IMGUIの更新
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		// バックバッファ（スワップチェイン）はFRAME_BUFFER_W/Hで固定作成されたまま、
		// ウィンドウの最大化・リサイズに合わせて動的に作り直されることはない。
		// ImGui_ImplWin32_NewFrame()は実際のクライアント領域をDisplaySizeに設定するが、
		// それをそのまま使うとImGuiが実際のバックバッファより大きい（あるいは小さい）
		// キャンバスがあるものとしてレイアウト・描画してしまい、見た目が崩れる。
		// 常にバックバッファと同じFRAME_BUFFER_W/Hに固定することで、ImGuiは常にバックバッファに
		// ぴったり収まるよう描画される。DXGIのPresentがバックバッファ全体をクライアント領域へ
		// 引き伸ばすため、結果的にゲーム本編と同じ比率でImGuiも一緒に拡大縮小表示される
		// （クリック判定側の変換はGame/Source/system/system.cppのWM_MOUSEMOVE処理で対応する）
		ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(FRAME_BUFFER_W), static_cast<float>(FRAME_BUFFER_H));

		ImGui::NewFrame();

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

		// ========== ImGui 描画 ==========
		ImGui::Render();

		// SRVヒープをセット
		auto* cmdList = g_graphicsEngine->GetCommandList();
		ID3D12DescriptorHeap* heaps[] = { m_imguiSrvHeap.Get() };
		cmdList->SetDescriptorHeaps(1, heaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

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