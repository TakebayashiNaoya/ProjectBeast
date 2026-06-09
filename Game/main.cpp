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

#ifdef DEBUG
	nsBeastEngine::nsCollision::PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif // DEBUG

	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		application->Update();

		auto* engine = nsBeastEngine::BeastEngine::GetInstance();
		engine->BeginExecute();
		application->Render(g_graphicsEngine->GetRenderContext());
		engine->EndExecute();
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