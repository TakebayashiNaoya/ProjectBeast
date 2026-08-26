#include "k2EngineLowPreCompile.h"
#include "Shader.h"
#include <stierr.h>
#include <sstream>
#include <fstream>
#include <atlbase.h>
#include <string>
#include <vector>

namespace nsK2EngineLow {

	namespace {
		const char* g_vsShaderModelName = "vs_5_0";	//頂点シェーダーのシェーダーモデル名。
		const char* g_psShaderModelName = "ps_5_0";	//ピクセルシェーダーのシェーダモデル名。
		const char* g_csShaderModelName = "cs_5_0";	//コンピュートシェーダーのシェーダーモデル名。

		//コンパイル済みシェーダーのディスクキャッシュの置き場所（作業ディレクトリ=Game/基準）。
		const char* g_shaderCacheDir = "ShaderCache";

		//指定フォルダを再帰的に走査して、含まれるファイルの最終更新時刻の最大値を求める。
		void ScanDirMaxWriteTime(const char* dir, long long& maxWriteTime)
		{
			char pattern[MAX_PATH];
			sprintf_s(pattern, "%s\\*", dir);
			WIN32_FIND_DATAA fd;
			HANDLE handle = FindFirstFileA(pattern, &fd);
			if (handle == INVALID_HANDLE_VALUE) {
				return;
			}
			do {
				if (fd.cFileName[0] == '.') {
					continue;
				}
				char path[MAX_PATH];
				sprintf_s(path, "%s\\%s", dir, fd.cFileName);
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					ScanDirMaxWriteTime(path, maxWriteTime);
				}
				else {
					ULARGE_INTEGER t;
					t.LowPart = fd.ftLastWriteTime.dwLowDateTime;
					t.HighPart = fd.ftLastWriteTime.dwHighDateTime;
					if (static_cast<long long>(t.QuadPart) > maxWriteTime) {
						maxWriteTime = static_cast<long long>(t.QuadPart);
					}
				}
			} while (FindNextFileA(handle, &fd));
			FindClose(handle);
		}

		//Assets/shader以下すべての最終更新時刻の最大値（プロセス起動後の初回だけ走査）。
		//どれか1ファイルでも編集されたら全キャッシュを無効化する。
		//fx本体のmtimeだけではincludeヘッダーの編集を検知できないため、フォルダ全体で見る。
		long long GetShaderDirMaxWriteTime()
		{
			static long long s_maxWriteTime = -1;
			if (s_maxWriteTime >= 0) {
				return s_maxWriteTime;
			}
			s_maxWriteTime = 0;
			ScanDirMaxWriteTime("Assets\\shader", s_maxWriteTime);
			return s_maxWriteTime;
		}

		//キャッシュファイルの先頭に置くヘッダー。
		struct ShaderCacheHeader
		{
			long long shaderDirTime;	//書き出した時点のAssets/shader以下の最終更新時刻の最大値。
			long long blobSize;			//後続のバイトコードのバイト数。壊れたキャッシュを弾く検証用。
		};

		//キャッシュファイルのパスを作る（fxパス・エントリ・モデル・コンパイルフラグで一意にする）。
		std::string BuildShaderCachePath(const char* filePath, const char* entryFuncName, const char* shaderModel, unsigned int compileFlags)
		{
			std::string name = filePath;
			for (auto& c : name) {
				if (c == '/' || c == '\\' || c == ':' || c == '.') {
					c = '_';
				}
			}
			char suffix[128];
			sprintf_s(suffix, "_%s_%s_%08x.cso", entryFuncName, shaderModel, compileFlags);
			return std::string(g_shaderCacheDir) + "/" + name + suffix;
		}
	}
	Shader::~Shader()
	{
		Release();
	}
	void Shader::Release()
	{
		ReleaseD3D12Object(m_blob);
		ReleaseD3D12Object(m_dxcBlob);
	}
	void Shader::Load(const char* filePath, const char* entryFuncName, const char* shaderModel)
	{
		Release();
		ID3DBlob* errorBlob;
#ifdef K2_DEBUG
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		//ディスクキャッシュにヒットしたら実行時コンパイルを丸ごとスキップする。
		//コンパイルは1シェーダー数百msかかり、ロード画面が数秒固まる主因だった。
		//シェーダーを1つでも編集するとフォルダ全体のmtimeが変わりキャッシュは無効になるので、
		//「.fxを直して再起動だけで反映」の開発フローは今までどおり使える。
		const long long shaderDirTime = GetShaderDirMaxWriteTime();
		const std::string cachePath = BuildShaderCachePath(filePath, entryFuncName, shaderModel, compileFlags);
		{
			std::ifstream cacheIn(cachePath, std::ios::binary);
			if (cacheIn) {
				ShaderCacheHeader header = {};
				cacheIn.read(reinterpret_cast<char*>(&header), sizeof(header));
				if (cacheIn && header.shaderDirTime == shaderDirTime && header.blobSize > 0) {
					std::vector<char> bytes(
						(std::istreambuf_iterator<char>(cacheIn)), std::istreambuf_iterator<char>());
					//記録しておいたサイズと照合し、書きかけのキャッシュは使わずコンパイルへ回す。
					//欠けたバイトコードをそのまま渡すとPSOの作成に失敗してabortし、
					//ShaderCacheを手で消すまで二度と起動できなくなる
					if (static_cast<long long>(bytes.size()) == header.blobSize
						&& SUCCEEDED(D3DCreateBlob(bytes.size(), &m_blob))) {
						memcpy(m_blob->GetBufferPointer(), bytes.data(), bytes.size());
						m_isInited = true;
						return;
					}
				}
			}
		}

		wchar_t wfxFilePath[256] = { L"" };
		mbstowcs(wfxFilePath, filePath, 256);

		auto hr = D3DCompileFromFile(wfxFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryFuncName, shaderModel, compileFlags, 0, &m_blob, &errorBlob);

		if (FAILED(hr)) {
			if (hr == STIERR_OBJECTNOTFOUND) {
				std::wstring errorMessage;
				errorMessage = L"指定されたfxファイルが開けませんでした。";
				errorMessage += wfxFilePath;
				MessageBoxW(nullptr, errorMessage.c_str(), L"エラー", MB_OK);
			}
			if (errorBlob) {
				static char errorMessage[10 * 1024];
				sprintf_s(errorMessage, "filePath : %ws, %s", wfxFilePath, (char*)errorBlob->GetBufferPointer());
				MessageBoxA(NULL, errorMessage, "シェーダーコンパイルエラー", MB_OK);
				return;
			}
		}

		//コンパイル結果をキャッシュへ保存する（次回以降の起動・ステージロードを高速化）。
		//いったん一時ファイルへ書いてから差し替える。本体へ直接書くと、書き込みの途中で
		//プロセスが落ちたとき（確保失敗のabort、ウィンドウを閉じる等）に中途半端な
		//キャッシュが残り、次回以降それを読み続けて起動できなくなる
		if (m_blob) {
			CreateDirectoryA(g_shaderCacheDir, nullptr);
			const std::string tempPath = cachePath + ".tmp";
			bool isWritten = false;
			{
				std::ofstream cacheOut(tempPath, std::ios::binary | std::ios::trunc);
				if (cacheOut) {
					ShaderCacheHeader header;
					header.shaderDirTime = shaderDirTime;
					header.blobSize = static_cast<long long>(m_blob->GetBufferSize());
					cacheOut.write(reinterpret_cast<const char*>(&header), sizeof(header));
					cacheOut.write(static_cast<const char*>(m_blob->GetBufferPointer()),
						static_cast<std::streamsize>(header.blobSize));
					cacheOut.flush();
					//ディスク不足などで書き切れていないものは差し替えない
					isWritten = cacheOut.good();
				}
			}
			if (isWritten) {
				MoveFileExA(tempPath.c_str(), cachePath.c_str(), MOVEFILE_REPLACE_EXISTING);
			}
			else {
				DeleteFileA(tempPath.c_str());
			}
		}
		m_isInited = true;
	}
	void Shader::LoadPS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_psShaderModelName);
	}
	void Shader::LoadVS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_vsShaderModelName);
	}
	void Shader::LoadCS(const char* filePath, const char* entryFuncName)
	{
		Load(filePath, entryFuncName, g_csShaderModelName);
	}
	void Shader::LoadRaytracing(const wchar_t* filePath)
	{
		std::ifstream shaderFile(filePath);
		if (shaderFile.good() == false) {
			std::wstring errormessage = L"シェーダーファイルのオープンに失敗しました。\n";
			errormessage += filePath;
			MessageBoxW(nullptr, errormessage.c_str(), L"エラー", MB_OK);
			std::abort();
		}

		std::stringstream strStream;
		strStream << shaderFile.rdbuf();
		std::string shader = strStream.str();
		//シェーダーのテキストファイルから、BLOBを作成する。
		CComPtr<IDxcLibrary> dxclib;
		auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxclib));
		if (FAILED(hr)) {
			MessageBox(nullptr, L"DXCLIBの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}
		CComPtr< IDxcIncludeHandler> includerHandler;
		hr = dxclib->CreateIncludeHandler(&includerHandler);
		if (FAILED(hr)) {
			MessageBox(nullptr, L"CreateIncludeHandlerに失敗しました。", L"エラー", MB_OK);
			std::abort();
		}

		//dxcコンパイラの作成。
		CComPtr<IDxcCompiler> dxcCompiler;
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		if (FAILED(hr)) {
			MessageBox(nullptr, L"dxcコンパイラの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}
		//ソースコードのBLOBを作成する。
		uint32_t codePage = CP_UTF8;
		CComPtr< IDxcBlobEncoding> sourceBlob;
		hr = dxclib->CreateBlobFromFile(filePath, &codePage, &sourceBlob);
		if (FAILED(hr)) {
			MessageBox(nullptr, L"シェーダーソースのBlobの作成に失敗しました。", L"エラー", MB_OK);
			std::abort();
		}

		CComPtr<IDxcIncludeHandler> dxcIncludeHandler;
		dxclib->CreateIncludeHandler(&dxcIncludeHandler);
		const wchar_t* args[] = {
			L"-I Assets\\shader",
		};
		//コンパイル。
		CComPtr<IDxcOperationResult> result;
		hr = dxcCompiler->Compile(
			sourceBlob, // pSource
			filePath, // pSourceName
			L"",		// pEntryPoint
			L"lib_6_3", // pTargetProfile
			args, 1, // pArguments, argCount
			nullptr, 0, // pDefines, defineCount
			dxcIncludeHandler, // pIncludeHandler
			&result); // ppResult
		if (SUCCEEDED(hr)) {
			result->GetStatus(&hr);
		}

		if (FAILED(hr))
		{
			if (result)
			{
				CComPtr<IDxcBlobEncoding> errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob)
				{
					std::string errormessage = "Compilation failed with errors:\n%hs\n";
					errormessage += (const char*)errorsBlob->GetBufferPointer();
					MessageBoxA(nullptr, errormessage.c_str(), "エラー", MB_OK);

				}
			}
			// Handle compilation error...
		}
		else {
			result->GetResult(&m_dxcBlob);
		}
	}
}
