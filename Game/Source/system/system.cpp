#include "stdafx.h"
#include "graphics/GraphicsEngine.h"
#include "sound/SoundEngine.h"
#include "system.h"
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()

HWND			g_hWnd = NULL;				//ウィンドウハンドル。

/** @brief ImGui Win32バックエンドのウィンドウメッセージハンドラ（前方宣言） */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

/**
 * @brief OSのバージョンに応じて適切なDPI Awareness APIを動的に選んで呼び出す（前方宣言）
 * @details 呼ばないままだとDPI非対応（Unaware）扱いになり、高DPI環境ではOSがウィンドウ全体を
 *          ビットマップ拡大縮小して表示する（DPI仮想化）。この状態だとマウス座標は仮想化された
 *          論理座標系のまま渡ってくる一方、実際の描画（スワップチェイン）は物理ピクセルで
 *          行われるため、ImGuiのボタン等の見た目とクリック判定がわずかにずれる原因になる。
 *          ウィンドウ作成前に呼ぶ必要がある。
 */
extern IMGUI_IMPL_API void ImGui_ImplWin32_EnableDpiAwareness();

/**
 * @brief WM_MOUSEMOVEのlParam（クライアント座標）を、実際のクライアント領域から
 *        固定デザイン解像度（FRAME_BUFFER_W x FRAME_BUFFER_H）へ変換したものに置き換える
 * @details バックバッファはFRAME_BUFFER_W/Hで固定のまま、ウィンドウの最大化・リサイズを
 *          許可しているため、DXGIがPresent時にバックバッファをクライアント領域へ合わせて
 *          自動的に引き伸ばして表示している。ImGui側もFRAME_BUFFER_W/Hを基準に描画しているため
 *          （BeastEngine::BeginExecute()参照）、クリック判定も同じ解像度に変換してから渡す必要がある
 */
LPARAM RemapMouseMoveLParamToDesignResolution(HWND hWnd, LPARAM lParam)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	const LONG clientWidth = clientRect.right - clientRect.left;
	const LONG clientHeight = clientRect.bottom - clientRect.top;
	if (clientWidth <= 0 || clientHeight <= 0) return lParam;

	const int x = GET_X_LPARAM(lParam);
	const int y = GET_Y_LPARAM(lParam);

	const int newX = static_cast<int>(x * (static_cast<float>(FRAME_BUFFER_W) / clientWidth));
	const int newY = static_cast<int>(y * (static_cast<float>(FRAME_BUFFER_H) / clientHeight));

	return MAKELPARAM(newX, newY);
}

///////////////////////////////////////////////////////////////////
//メッセージプロシージャ。
//hWndがメッセージを送ってきたウィンドウのハンドル。
//msgがメッセージの種類。
//wParamとlParamは引数。今は気にしなくてよい。
///////////////////////////////////////////////////////////////////
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_MOUSEMOVE)
	{
		lParam = RemapMouseMoveLParamToDesignResolution(hWnd, lParam);
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return 1;

	//送られてきたメッセージで処理を分岐させる。
	switch (msg)
	{
	case WM_DESTROY:
		//エンジンの破棄。
		// ゲームオブジェクトマネージャーの更新処理を呼び出す。
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////
// ウィンドウの初期化。
///////////////////////////////////////////////////////////////////
void InitWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName)
{
	// DPI仮想化によるマウス座標とレンダリング結果のズレを防ぐため、ウィンドウ作成前にDPI対応を有効化する
	ImGui_ImplWin32_EnableDpiAwareness();

	//ウィンドウクラスのパラメータを設定(単なる構造体の変数の初期化です。)
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),		//構造体のサイズ。
		CS_CLASSDC,				//ウィンドウのスタイル。
		//ここの指定でスクロールバーをつけたりできるが、ゲームでは不要なのでCS_CLASSDCでよい。
MsgProc,				//メッセージプロシージャ(後述)
0,						//0でいい。
0,						//0でいい。
GetModuleHandle(NULL),	//このクラスのためのウインドウプロシージャがあるインスタンスハンドル。
//何も気にしなくてよい。
NULL,					//アイコンのハンドル。アイコンを変えたい場合ここを変更する。とりあえずこれでいい。
NULL,					//マウスカーソルのハンドル。NULLの場合はデフォルト。
NULL,					//ウィンドウの背景色。NULLの場合はデフォルト。
NULL,					//メニュー名。NULLでいい。
appName,				//ウィンドウクラスに付ける名前。
NULL					//NULLでいい。
	};
	//ウィンドウクラスの登録。
	RegisterClassEx(&wc);

	// バックバッファ（スワップチェイン）はFRAME_BUFFER_W/Hで固定作成された後、一切リサイズされない
	// （WM_SIZEでバッファを作り直す処理が存在しない）。ウィンドウの最大化・リサイズ自体は許可し、
	// クライアント領域とバックバッファ解像度の食い違いはBeastEngine::BeginExecute()側で
	// 吸収する（ImGuiを常にFRAME_BUFFER_W/Hの固定解像度で描画し、マウス座標もそれに合わせて
	// 変換する。DXGIのPresentがバックバッファ全体をクライアント領域へ引き伸ばすため、
	// 結果的にゲーム本編と同じ比率でImGuiも一緒に拡大表示される）

	// FRAME_BUFFER_W x FRAME_BUFFER_H は「クライアント領域（描画可能な内側）」のサイズとして
	// 扱いたいが、CreateWindowに渡すサイズはタイトルバー・枠を含む「ウィンドウ全体」のサイズになる。
	// AdjustWindowRectで「クライアント領域がFRAME_BUFFER_W/Hになる」ような外寸を逆算する
	// （起動直後の初期表示サイズを正確に合わせるため。最大化・リサイズ後は上記の変換で対応する）
	RECT windowRect = { 0, 0, static_cast<LONG>(FRAME_BUFFER_W), static_cast<LONG>(FRAME_BUFFER_H) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウの作成。
	g_hWnd = CreateWindow(
		appName,				//使用するウィンドウクラスの名前。
		//先ほど作成したウィンドウクラスと同じ名前にする。
		appName,				//ウィンドウの名前。ウィンドウクラスの名前と別名でもよい。
		WS_OVERLAPPEDWINDOW,	//ウィンドウスタイル。ゲームでは基本的にWS_OVERLAPPEDWINDOWでいい、
		0,						//ウィンドウの初期X座標。
		0,						//ウィンドウの初期Y座標。
		windowRect.right - windowRect.left,	//ウィンドウの幅（クライアント領域がFRAME_BUFFER_Wになる外寸）。
		windowRect.bottom - windowRect.top,	//ウィンドウの高さ（クライアント領域がFRAME_BUFFER_Hになる外寸）。
		NULL,					//親ウィンドウ。ゲームでは基本的にNULLでいい。
		NULL,					//メニュー。今はNULLでいい。
		hInstance,				//アプリケーションのインスタンス。
		NULL
	);

	ShowWindow(g_hWnd, nCmdShow);

}


//ゲームの初期化。
void InitGame(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow, const TCHAR* appName)
{
	//ウィンドウを初期化。
	InitWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow, appName);
	//k2エンジンの初期化。
	nsBeastEngine::BeastEngine::InitData initData;
	initData.frameBufferWidth = FRAME_BUFFER_W;
	initData.frameBufferHeight = FRAME_BUFFER_H;
	initData.hwnd = g_hWnd;
	nsBeastEngine::BeastEngine::CreateInstance(initData);
}
//ウィンドウメッセージをディスパッチ。falseが返ってきたら、ゲーム終了。
bool DispatchWindowMessage()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		//ウィンドウからのメッセージを受け取る。
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//ウィンドウメッセージが空になった。
			break;
		}
	}
	return msg.message != WM_QUIT;
}
