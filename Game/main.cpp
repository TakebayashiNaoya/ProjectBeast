#include "stdafx.h"
#include "Source/system/system.h"

#include<dxgidebug.h>
#include<InitGUID.h>
#include<time.h>

#include "Source/Application.h"



void ReportLiveObjects()
{
	IDXGIDebug* pDxgiDebug;

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);

	// 出力。
	pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
}

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数。
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//ゲームの初期化。
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
	//////////////////////////////////////
	// ここから初期化を行うコードを記述する。
	//////////////////////////////////////


	srand(time(nullptr));
	app::Application* application = new app::Application();

	//////////////////////////////////////
	// 初期化を行うコードを書くのはここまで！！！
	//////////////////////////////////////
	nsBeastEngine::nsCollision::PhysicsWorld::Initialize();

#ifdef DEBUG
	nsBeastEngine::nsCollision::PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif // DEBUG

	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		g_engine->BeginFrame();

		// 更新まわりはここから下

		g_engine->ExecuteUpdate();
		application->Update();

		// 描画まわりはここから下

		// レンダリングエンジンの更新。
		g_renderingEngine->Update();

		g_engine->ExecuteRender();
		auto& renderContext = g_graphicsEngine->GetRenderContext();

#ifdef DEBUG
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DebubDrawWorld(renderContext);
#endif // DEBUG

		application->Render(renderContext);
		//レンダリングエンジンを実行。		
		g_renderingEngine->Execute(renderContext);

		//当たり判定描画。
		g_engine->DebubDrawWorld();



		g_engine->EndFrame();
	}

	delete application;

	/**
	 * ゲームオブジェクトの破棄
	 */
	 //delete game;

	nsBeastEngine::BeastEngine::DeleteInstance();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}

