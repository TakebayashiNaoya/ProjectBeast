#include "k2EngineLowPreCompile.h"
#include "GraphicsEngine.h"
#include <pix.h>

namespace nsK2EngineLow {
	GraphicsEngine* g_graphicsEngine = nullptr;	//グラフィックスエンジン
	Camera* g_camera2D = nullptr;				//2Dカメラ。
	Camera* g_camera3D = nullptr;				//3Dカメラ。

	GraphicsEngine::~GraphicsEngine()
	{
		WaitDraw();

		for (auto& req : m_reqDelayRelease3d12ObjectList) {
			if (req.d3dObject) {
				req.d3dObject->Release();
			}
		}
		//後始末。
		if (m_commandQueue) {
			m_commandQueue->Release();
		}


		m_frameBuffer.Release();

		for (auto& commandAllocator : m_commandAllocator) {
			if (commandAllocator) {
				commandAllocator->Release();
			}
		}
		for (auto& commandList : m_commandList) {
			if (commandList) {
				commandList->Release();
			}
		}
		if (m_pipelineState) {
			m_pipelineState->Release();
		}

		if (m_fence) {
			m_fence->Release();
		}

		if (m_d3dDevice) {
			m_d3dDevice->Release();
		}
		
		CloseHandle(m_fenceEvent);
	}
	void GraphicsEngine::WaitDraw()
	{

		//描画終了待ち
		// Signal and increment the fence value.
		const UINT64 fence = m_fenceValue;
		m_commandQueue->Signal(m_fence, fence);
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue() < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}
	bool GraphicsEngine::Init(HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
	{
		//
		g_graphicsEngine = this;

		m_frameBufferWidth = frameBufferWidth;
		m_frameBufferHeight = frameBufferHeight;

		//デバイスにアクセスするためのインターフェースを作成。
		auto dxgiFactory = CreateDXGIFactory();

		// Enable DRED (Device Removed Extended Data) before creating the device,
		// so that we can find out why the device was removed (GPU hang, page fault, etc.).
		{
			ID3D12DeviceRemovedExtendedDataSettings* dredSettings = nullptr;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)))) {
				dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				dredSettings->Release();
			}
			// Breadcrumb contexts record marker/event strings next to each breadcrumb op,
			// which lets the report name the render pass and model around a GPU hang.
			ID3D12DeviceRemovedExtendedDataSettings1* dredSettings1 = nullptr;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings1)))) {
				dredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				dredSettings1->Release();
			}
		}

		//D3Dデバイスの作成。
		if (!CreateD3DDevice(dxgiFactory)) {
			//D3Dデバイスの作成に失敗した。
			MessageBox(hwnd, TEXT("D3Dデバイスの作成に失敗しました。"), TEXT("エラー"), MB_OK);
			return false;
		}
		//コマンドキューの作成。
		if (!CreateCommandQueue()) {
			//コマンドキューの作成に失敗した。
			MessageBox(hwnd, TEXT("コマンドキューの作成に失敗しました。"), TEXT("エラー"), MB_OK);
			return false;
		}

		//フレームバッファを初期化
		if (!m_frameBuffer.Init(hwnd, m_d3dDevice, m_commandQueue, dxgiFactory, frameBufferWidth, frameBufferHeight)) {
			MessageBox(hwnd, TEXT("フレームバッファの作成に失敗しました。"), TEXT("エラー"), MB_OK);
			return false;
		}

		//コマンドアロケータの作成。
		for (auto& commandAllocator : m_commandAllocator) {
			m_d3dDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&commandAllocator));

			if (!commandAllocator) {
				MessageBox(hwnd, TEXT("コマンドアロケータの作成に失敗しました。"), TEXT("エラー"), MB_OK);
				return false;
			}
		}

		//コマンドリストの作成。
		if (!CreateCommandList()) {
			MessageBox(hwnd, TEXT("コマンドリストの作成に失敗しました。"), TEXT("エラー"), MB_OK);
			return false;
		}

		//GPUと同期をとるためのオブジェクトを作成する。
		if (!CreateSynchronizationWithGPUObject()) {
			MessageBox(hwnd, TEXT("GPUと同期をとるためのオブジェクトの作成に失敗しました。"), TEXT("エラー"), MB_OK);
			return false;
		}

		//レンダリングコンテキストの作成。
		m_renderContext.Init(m_commandList[m_frameIndex]);


		//CBR_SVRのディスクリプタのサイズを取得。
		m_cbrSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//Samplerのディスクリプタのサイズを取得。
		m_samplerDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		//初期化が終わったのでDXGIを破棄。
		dxgiFactory->Release();

		//ヌルテクスチャを初期化
		m_nullTextureMaps.Init();

		//カメラを初期化する。
		m_camera2D.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
		m_camera2D.SetWidth(static_cast<float>(m_frameBufferWidth));
		m_camera2D.SetHeight(static_cast<float>(m_frameBufferHeight));
		m_camera2D.SetPosition({ 0.0f, 0.0f, -1.0f });
		m_camera2D.SetTarget({ 0.0f, 0.0f, 0.0f });

		m_camera3D.SetPosition({ 0.0f, 50.0f, -200.0f });
		m_camera3D.SetTarget({ 0.0f, 50.0f, 0.0f });

		g_camera2D = &m_camera2D;
		g_camera3D = &m_camera3D;

		//DirectXTK用のグラフィッsクメモリ管理クラスのインスタンスを作成する。
		m_directXTKGfxMemroy = std::make_unique<DirectX::GraphicsMemory>(m_d3dDevice);
		//フォント描画エンジンを初期化。
		m_fontEngine.Init();

		return true;
	}

	IDXGIFactory4* GraphicsEngine::CreateDXGIFactory()
	{
		UINT dxgiFactoryFlags = 0;
#ifdef K2_DEBUG
		//デバッグコントローラーがあれば、デバッグレイヤーがあるDXGIを作成する。
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			debugController->Release();
		}
#endif
		IDXGIFactory4* factory;
		CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
		return factory;
	}

	bool GraphicsEngine::CreateD3DDevice(IDXGIFactory4* dxgiFactory)
	{
		D3D_FEATURE_LEVEL featureLevels[] = {
				D3D_FEATURE_LEVEL_12_1,	//Direct3D 12.1の機能を使う。
				D3D_FEATURE_LEVEL_12_0	//Direct3D 12.0の機能を使う。
		};
		IDXGIAdapter* adapterTmp = nullptr;
		IDXGIAdapter* adapterVender[Num_GPUVender] = { nullptr };	//各ベンダーのアダプター。
		IDXGIAdapter* adapterMaxVideoMemory = nullptr;				//最大ビデオメモリのアダプタ。
		IDXGIAdapter* useAdapter = nullptr;							//最終的に使用するアダプタ。
		SIZE_T videoMemorySize = 0;
		for (int i = 0; dxgiFactory->EnumAdapters(i, &adapterTmp) != DXGI_ERROR_NOT_FOUND; i++) {
			DXGI_ADAPTER_DESC desc;
			adapterTmp->GetDesc(&desc);
			if (desc.DedicatedVideoMemory > videoMemorySize) {
				//こちらのビデオメモリの方が多いので、こちらを使う。
				if (adapterMaxVideoMemory != nullptr) {
					adapterMaxVideoMemory->Release();
				}
				adapterMaxVideoMemory = adapterTmp;
				adapterMaxVideoMemory->AddRef();
				videoMemorySize = desc.DedicatedVideoMemory;
			}
			if (wcsstr(desc.Description, L"NVIDIA") != nullptr) {
				//NVIDIA製
				if (adapterVender[GPU_VenderNvidia]) {
					adapterVender[GPU_VenderNvidia]->Release();
				}
				adapterVender[GPU_VenderNvidia] = adapterTmp;
				adapterVender[GPU_VenderNvidia]->AddRef();
			}
			else if (wcsstr(desc.Description, L"AMD") != nullptr) {
				//AMD製
				if (adapterVender[GPU_VenderAMD]) {
					adapterVender[GPU_VenderAMD]->Release();
				}
				adapterVender[GPU_VenderAMD] = adapterTmp;
				adapterVender[GPU_VenderAMD]->AddRef();
			}
			else if (wcsstr(desc.Description, L"Intel") != nullptr) {
				//Intel製
				if (adapterVender[GPU_VenderIntel]) {
					adapterVender[GPU_VenderIntel]->Release();
				}
				adapterVender[GPU_VenderIntel] = adapterTmp;
				adapterVender[GPU_VenderIntel]->AddRef();
			}
			adapterTmp->Release();
		}
		//使用するアダプターを決める。
		if (adapterVender[GPU_VenderNvidia] != nullptr) {
			//NVIDIA製が最優先
			useAdapter = adapterVender[GPU_VenderNvidia];
		}
		else if (adapterVender[GPU_VenderAMD] != nullptr) {
			//次はAMDが優先。
			useAdapter = adapterVender[GPU_VenderAMD];
		}
		else {
			//NVIDIAとAMDのGPUがなければビデオメモリが一番多いやつを使う。
			useAdapter = adapterMaxVideoMemory;
		}
		for (auto featureLevel : featureLevels) {
			auto hr = D3D12CreateDevice(
				useAdapter,
				featureLevel,
				IID_PPV_ARGS(&m_d3dDevice)
			);
			if (SUCCEEDED(hr)) {
				//D3Dデバイスの作成に成功した。
				break;
			}
		}
		for (auto& adapter : adapterVender) {
			if (adapter) {
				adapter->Release();
			}
		}
		if (adapterMaxVideoMemory) {
			adapterMaxVideoMemory->Release();
		}
#ifdef K2_DEBUG
		// Windows11の不具合の回避対応
		// Windows11でMISMATCHING_COMMAND_LIST_TYPEが出るようになっており、
		// WindowsSDKの不具合とのこと。
		// 近日中に修正されるらしい。
		// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
		ID3D12InfoQueue* infoQueue;
		if ( m_d3dDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)) == S_OK ) {
			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
				// Workarounds for debug layer issues on hybrid-graphics systems
				D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
				D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
			};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			infoQueue->AddStorageFilterEntries(&filter);
			infoQueue->Release();
		}
#endif
		return m_d3dDevice != nullptr;
	}
	bool GraphicsEngine::CreateCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		auto hr = m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
		if (FAILED(hr)) {
			//コマンドキューの作成に失敗した。
			return false;
		}
		// Name the queue so DRED breadcrumbs can identify it after a device removal.
		m_commandQueue->SetName(L"MainCommandQueue");

		return true;
	}

	bool GraphicsEngine::CreateCommandList()
	{
		int listNo = 0;
		for (auto& commandList : m_commandList) {
			//コマンドリストの作成。
			m_d3dDevice->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_commandAllocator[listNo],
				nullptr, IID_PPV_ARGS(&commandList)
			);
			if (!commandList) {
				return false;
			}
			// Name the list so DRED breadcrumbs can identify it after a device removal.
			wchar_t listName[32];
			swprintf_s(listName, L"MainCommandList%d", listNo);
			commandList->SetName(listName);
			//コマンドリストは開かれている状態で作成されるので、いったん閉じる。
			commandList->Close();

			listNo++;
		}
		return true;
	}
	bool GraphicsEngine::CreateSynchronizationWithGPUObject()
	{
		m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		if (!m_fence) {
			//フェンスの作成に失敗した。
			return false;
		}
		m_fenceValue = 1;
		//同期を行うときのイベントハンドラを作成する。
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr) {
			return false;
		}
		return true;
	}
	void GraphicsEngine::BeginRender()
	{
		// Catch a device removal that happened during the game update phase,
		// before any draw call touches the broken device.
		if (m_d3dDevice && FAILED(m_d3dDevice->GetDeviceRemovedReason())) {
			ReportDeviceRemoved("GraphicsEngine::BeginRender device check");
		}

		m_frameIndex = m_frameBuffer.GetCurrentBackBufferIndex();

		//カメラを更新する。
		m_camera2D.Update();
		m_camera3D.Update();

		//コマンドアロケータををリセット。
		m_commandAllocator[m_frameIndex]->Reset();

		m_renderContext.SetCommandList(m_commandList[m_frameIndex]);
		//レンダリングコンテキストもリセット。
		m_renderContext.Reset(m_commandAllocator[m_frameIndex], m_pipelineState);


		//フレームバッファをレンダリングターゲットとして設定可能になるまで待つ。
		m_renderContext.WaitUntilToPossibleSetRenderTarget(m_frameBuffer.GetCurrentRenderTarget());

		//フレームバッファをレンダリングターゲットを設定。
		m_renderContext.SetRenderTarget(
			m_frameBuffer.GetCurrentRenderTargetViewDescriptorHandle(),
			m_frameBuffer.GetCurrentDepthStencilViewDescriptorHandle()
		);
		m_renderContext.SetViewportAndScissor(m_frameBuffer.GetViewport());
		//フレームバッファをクリア。
		const float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		m_renderContext.ClearRenderTargetView(m_frameBuffer.GetCurrentRenderTargetViewDescriptorHandle(), clearColor);
		m_renderContext.ClearDepthStencilView(m_frameBuffer.GetCurrentDepthStencilViewDescriptorHandle(), 1.0f);

	}
	void GraphicsEngine::ChangeRenderTargetToFrameBuffer(RenderContext& rc)
	{
		rc.SetRenderTarget(
			m_frameBuffer.GetCurrentRenderTargetViewDescriptorHandle(),
			m_frameBuffer.GetCurrentDepthStencilViewDescriptorHandle()
		);
	}
	void GraphicsEngine::ExecuteRequestReleaseD3D12Object()
	{
		auto releaseReqIt = m_reqDelayRelease3d12ObjectList.begin();
		while (releaseReqIt != m_reqDelayRelease3d12ObjectList.end()) {
			if (releaseReqIt->delayTime == 0) {
				// 開放
				if (releaseReqIt->d3dObject) {
					releaseReqIt->d3dObject->Release();
				}
				releaseReqIt = m_reqDelayRelease3d12ObjectList.erase(releaseReqIt);
			}
			else {
				releaseReqIt->delayTime--;
				releaseReqIt++;
			}
		}
	}
	void GraphicsEngine::EndRender()
	{
		// If the device has been removed, report the reason and stop here
		// instead of crashing somewhere else with a broken device.
		if (m_d3dDevice && FAILED(m_d3dDevice->GetDeviceRemovedReason())) {
			ReportDeviceRemoved("GraphicsEngine::EndRender device check");
		}

		// レンダリングターゲットへの描き込み完了待ち
		m_renderContext.WaitUntilFinishDrawingToRenderTarget(m_frameBuffer.GetCurrentRenderTarget());
		
		if (m_isExecuteCommandList)
		{
#ifdef USE_FPS_LIMITTER
			m_frameBuffer.Present(1);
#else
			m_frameBuffer.Present(0);
#endif
			// 描画完了待ち。
			WaitDraw();
		}

		m_directXTKGfxMemroy->Commit(m_commandQueue);
		//レンダリングコンテキストを閉じる。
		m_renderContext.Close();
		//このフレームに作成した描画コマンドを実行する。
		ID3D12CommandList* ppCommandLists[] = { m_commandList[m_frameIndex] };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// コマンドリストをGPUに流した印。
		m_isExecuteCommandList = true;
		m_directXTKGfxMemroy->GarbageCollect();

		// バックバッファを入れ替える。
		m_frameBuffer.SwapBackBuffer();

		// D3D12オブジェクトの解放リクエストを処理する。
		ExecuteRequestReleaseD3D12Object();

	}

	namespace {
		FILE* g_dredLogFile = nullptr;
		// Print to both the log file and the debugger output window.
		void DredLog(const char* fmt, ...)
		{
			char buf[2048];
			va_list args;
			va_start(args, fmt);
			vsprintf_s(buf, fmt, args);
			va_end(args);
			if (g_dredLogFile) {
				fputs(buf, g_dredLogFile);
			}
			OutputDebugStringA(buf);
		}

		// Readable names for D3D12_AUTO_BREADCRUMB_OP values.
		const char* GetBreadcrumbOpName(int op)
		{
			switch (op) {
			case 0:  return "SETMARKER";
			case 1:  return "BEGINEVENT";
			case 2:  return "ENDEVENT";
			case 3:  return "DRAWINSTANCED";
			case 4:  return "DRAWINDEXEDINSTANCED";
			case 5:  return "EXECUTEINDIRECT";
			case 6:  return "DISPATCH";
			case 7:  return "COPYBUFFERREGION";
			case 8:  return "COPYTEXTUREREGION";
			case 9:  return "COPYRESOURCE";
			case 10: return "COPYTILES";
			case 11: return "RESOLVESUBRESOURCE";
			case 12: return "CLEARRENDERTARGETVIEW";
			case 13: return "CLEARUNORDEREDACCESSVIEW";
			case 14: return "CLEARDEPTHSTENCILVIEW";
			case 15: return "RESOURCEBARRIER";
			case 16: return "EXECUTEBUNDLE";
			case 17: return "PRESENT";
			case 18: return "RESOLVEQUERYDATA";
			case 19: return "BEGINSUBMISSION";
			case 20: return "ENDSUBMISSION";
			case 26: return "WRITEBUFFERIMMEDIATE";
			case 39: return "SETPIPELINESTATE1";
			case 42: return "DISPATCHMESH";
			case 45: return "BARRIER";
			case 46: return "BEGIN_COMMAND_LIST";
			default: return "?";
			}
		}
	}

	void GraphicsEngine::QueryVideoMemoryMB(double& localUsageMB, double& localBudgetMB)
	{
		localUsageMB = 0.0;
		localBudgetMB = 0.0;
		if (m_d3dDevice == nullptr) {
			return;
		}
		IDXGIFactory4* factory = nullptr;
		if (SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
			IDXGIAdapter3* adapter = nullptr;
			const LUID luid = m_d3dDevice->GetAdapterLuid();
			if (SUCCEEDED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&adapter)))) {
				DXGI_QUERY_VIDEO_MEMORY_INFO local = {};
				adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local);
				localUsageMB = local.CurrentUsage / (1024.0 * 1024.0);
				localBudgetMB = local.Budget / (1024.0 * 1024.0);
				adapter->Release();
			}
			factory->Release();
		}
	}

	// DescriptorHeap.cpp のヒープ数カウンタ（デバイスロストレポートに載せる用）
	extern int g_numDescriptorHeap;
	extern int g_numDescriptorHeapLive;

	void GraphicsEngine::ReportDeviceRemoved(const char* site)
	{
		// Model loading can run on a worker thread, so two threads may detect the
		// removal at the same time. Let the first one write the report and abort;
		// park any other caller here forever.
		static std::atomic<bool> s_isReporting = false;
		if (s_isReporting.exchange(true)) {
			Sleep(INFINITE);
		}

		const HRESULT reason = m_d3dDevice ? m_d3dDevice->GetDeviceRemovedReason() : E_FAIL;

		// The working directory is Game/, so this lands next to the play logs.
		char logPath[256];
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf_s(logPath, "Logs/device_removed_%04d-%02d-%02d_%02d-%02d-%02d.txt",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		fopen_s(&g_dredLogFile, logPath, "w");
		if (g_dredLogFile == nullptr) {
			fopen_s(&g_dredLogFile, "device_removed.txt", "w");
		}

		const char* reasonStr = "unknown";
		switch (reason) {
		case DXGI_ERROR_DEVICE_HUNG:            reasonStr = "DXGI_ERROR_DEVICE_HUNG (GPU hang / TDR timeout)"; break;
		case DXGI_ERROR_DEVICE_REMOVED:         reasonStr = "DXGI_ERROR_DEVICE_REMOVED (adapter removed)"; break;
		case DXGI_ERROR_DEVICE_RESET:           reasonStr = "DXGI_ERROR_DEVICE_RESET"; break;
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR:  reasonStr = "DXGI_ERROR_DRIVER_INTERNAL_ERROR (often VRAM exhaustion)"; break;
		case DXGI_ERROR_INVALID_CALL:           reasonStr = "DXGI_ERROR_INVALID_CALL"; break;
		case S_OK:                              reasonStr = "S_OK (device is NOT removed)"; break;
		}
		DredLog("=== Device Removed Report ===\n");
		// Which call site triggered the report. When the reason below is S_OK, the
		// device is fine and the real problem is an allocation failure at this site.
		DredLog("Report site: %s\n", site ? site : "(unknown)");
		DredLog("GetDeviceRemovedReason: 0x%08X %s\n", static_cast<unsigned int>(reason), reasonStr);
		// Cumulative shader-visible descriptor heap creations. Drivers can fail
		// CreateDescriptorHeap when too many shader-visible heaps are alive.
		DredLog("DescriptorHeap creations so far: %d (alive now: %d)\n",
			g_numDescriptorHeap, g_numDescriptorHeapLive);

		// Current video memory usage of the adapter this device runs on.
		{
			IDXGIFactory4* factory = nullptr;
			if (SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
				IDXGIAdapter3* adapter = nullptr;
				const LUID luid = m_d3dDevice->GetAdapterLuid();
				if (SUCCEEDED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&adapter)))) {
					DXGI_ADAPTER_DESC desc;
					adapter->GetDesc(&desc);
					DredLog("Adapter: %ls\n", desc.Description);
					DXGI_QUERY_VIDEO_MEMORY_INFO local = {};
					DXGI_QUERY_VIDEO_MEMORY_INFO nonLocal = {};
					adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local);
					adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocal);
					DredLog("VRAM local    : usage %.1f MB / budget %.1f MB\n",
						local.CurrentUsage / (1024.0 * 1024.0), local.Budget / (1024.0 * 1024.0));
					DredLog("VRAM non-local: usage %.1f MB / budget %.1f MB\n",
						nonLocal.CurrentUsage / (1024.0 * 1024.0), nonLocal.Budget / (1024.0 * 1024.0));
					adapter->Release();
				}
				factory->Release();
			}
		}

		// DRED breadcrumbs: which command list stopped at which operation.
		// Use DRED 1.1 so marker/event strings (render pass and model names) appear inline.
		ID3D12DeviceRemovedExtendedData1* dred1 = nullptr;
		if (m_d3dDevice && SUCCEEDED(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&dred1)))) {
			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs = {};
			if (SUCCEEDED(dred1->GetAutoBreadcrumbsOutput1(&breadcrumbs))) {
				DredLog("--- Auto breadcrumbs (only command lists that did not finish) ---\n");
				for (auto* node = breadcrumbs.pHeadAutoBreadcrumbNode; node != nullptr; node = node->pNext) {
					const UINT32 last = node->pLastBreadcrumbValue ? *node->pLastBreadcrumbValue : 0;
					if (last == node->BreadcrumbCount) {
						continue;
					}
					// Prefer the ANSI debug name; fall back to the wide name set via SetName().
					char listName[64] = "(no name)";
					char queueName[64] = "(no name)";
					if (node->pCommandListDebugNameA) {
						sprintf_s(listName, "%s", node->pCommandListDebugNameA);
					} else if (node->pCommandListDebugNameW) {
						sprintf_s(listName, "%ls", node->pCommandListDebugNameW);
					}
					if (node->pCommandQueueDebugNameA) {
						sprintf_s(queueName, "%s", node->pCommandQueueDebugNameA);
					} else if (node->pCommandQueueDebugNameW) {
						sprintf_s(queueName, "%ls", node->pCommandQueueDebugNameW);
					}
					DredLog("CommandList '%s' on queue '%s': stopped at op %u / %u\n",
						listName, queueName, last, node->BreadcrumbCount);
					// Dump the whole op history with any marker/event strings so the
					// render pass and model around the hang can be identified.
					UINT32 ctxIdx = 0;
					for (UINT32 i = 0; i < node->BreadcrumbCount; i++) {
						const int op = static_cast<int>(node->pCommandHistory[i]);
						const wchar_t* context = nullptr;
						while (ctxIdx < node->BreadcrumbContextsCount &&
							node->pBreadcrumbContexts[ctxIdx].BreadcrumbIndex < i) {
							ctxIdx++;
						}
						if (ctxIdx < node->BreadcrumbContextsCount &&
							node->pBreadcrumbContexts[ctxIdx].BreadcrumbIndex == i) {
							context = node->pBreadcrumbContexts[ctxIdx].pContextString;
						}
						DredLog("  op[%u] = %d %s%s%ls%s%s\n", i, op, GetBreadcrumbOpName(op),
							context ? "  '" : "", context ? context : L"", context ? "'" : "",
							(i == last) ? "  <-- last executed" : "");
					}
				}
			}
			dred1->Release();
		}
		// Fallback: DRED 1.0 without contexts.
		ID3D12DeviceRemovedExtendedData* dred = nullptr;
		if (dred1 == nullptr && m_d3dDevice && SUCCEEDED(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&dred)))) {
			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};
			if (SUCCEEDED(dred->GetAutoBreadcrumbsOutput(&breadcrumbs))) {
				DredLog("--- Auto breadcrumbs (DRED 1.0, no contexts) ---\n");
				for (auto* node = breadcrumbs.pHeadAutoBreadcrumbNode; node != nullptr; node = node->pNext) {
					const UINT32 last = node->pLastBreadcrumbValue ? *node->pLastBreadcrumbValue : 0;
					if (last == node->BreadcrumbCount) {
						continue;
					}
					DredLog("CommandList '%s': stopped at op %u / %u\n",
						node->pCommandListDebugNameA ? node->pCommandListDebugNameA : "(no name)",
						last, node->BreadcrumbCount);
					for (UINT32 i = 0; i < node->BreadcrumbCount; i++) {
						const int op = static_cast<int>(node->pCommandHistory[i]);
						DredLog("  op[%u] = %d %s%s\n", i, op, GetBreadcrumbOpName(op),
							(i == last) ? "  <-- last executed" : "");
					}
				}
			}
			dred->Release();
		}

		// Page fault info comes from the base DRED interface; report it in both paths.
		ID3D12DeviceRemovedExtendedData* dredPf = nullptr;
		if (m_d3dDevice && SUCCEEDED(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&dredPf)))) {
			D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
			if (SUCCEEDED(dredPf->GetPageFaultAllocationOutput(&pageFault))) {
				DredLog("--- Page fault ---\n");
				DredLog("PageFaultVA: 0x%llX\n", static_cast<unsigned long long>(pageFault.PageFaultVA));
				for (auto* n = pageFault.pHeadExistingAllocationNode; n != nullptr; n = n->pNext) {
					DredLog("existing allocation: '%s' type %d\n",
						n->ObjectNameA ? n->ObjectNameA : "(no name)", static_cast<int>(n->AllocationType));
				}
				for (auto* n = pageFault.pHeadRecentFreedAllocationNode; n != nullptr; n = n->pNext) {
					DredLog("recently freed allocation: '%s' type %d\n",
						n->ObjectNameA ? n->ObjectNameA : "(no name)", static_cast<int>(n->AllocationType));
				}
			}
			dredPf->Release();
		}

		DredLog("=== End of report ===\n");
		if (g_dredLogFile) {
			fclose(g_dredLogFile);
			g_dredLogFile = nullptr;
		}

		wchar_t msg[512];
		swprintf_s(msg,
			L"GPU device removed (device lost) was detected.\nA report was written to:\n%hs",
			logPath);
		MessageBox(nullptr, msg, L"GPU Device Removed", MB_OK);
		std::abort();
	}
}
