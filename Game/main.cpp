#include "stdafx.h"
#include "system/system.h"

#include<dxgidebug.h>
#include<InitGUID.h>

#include "Test.h"


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


	/** 当たり判定を可視化 */
	PhysicsWorld::GetInstance()->EnableDrawDebugWireFrame();

	/**
	 * ゲームオブジェクトの生成
	 */
	auto* game = NewGO<Test>(0);

	//////////////////////////////////////
	// 初期化を行うコードを書くのはここまで！！！
	//////////////////////////////////////

	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		nsBeastEngine::BeastEngine::GetInstance()->Execute();
	}

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

