/**
 * @file VideoClip.cpp
 * @brief コマ撮り／映像フレームデータ管理クラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "VideoClip.h"
#include <algorithm>
#include <cctype>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Ole32.lib")

 // stb_image ラッパー関数の宣言（stb_image_impl.cpp で定義）
unsigned char* beast_stbi_load(const char* filename, int* x, int* y, int* channels, int desired);
void           beast_stbi_free(void* ptr);


namespace nsBeastEngine
{
	static std::string GetLowerExtension(const std::string& path)
	{
		const auto dot = path.rfind('.');
		if (dot == std::string::npos) return "";
		std::string ext = path.substr(dot);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext;
	}

	static std::vector<std::string> CollectImageFiles(const std::string& folderPath)
	{
		std::vector<std::string> files;
		const std::string searchPath = folderPath + "*";

		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE) return files;

		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			const std::string ext = GetLowerExtension(findData.cFileName);
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
			{
				files.push_back(folderPath + findData.cFileName);
			}
		} while (FindNextFileA(hFind, &findData));

		FindClose(hFind);
		std::sort(files.begin(), files.end());
		return files;
	}


	VideoClip::~VideoClip()
	{
		CleanupMP4();
	}


	void VideoClip::Unload()
	{
		CleanupMP4();
		m_frames.clear();
		m_mp4FrameBuffer.clear();
		m_width          = 0;
		m_height         = 0;
		m_frameCount     = 0;
		m_fps            = 24.0f;
		m_clipType       = ClipType::None;
		m_mp4CurrentFrame = -1;
		m_mp4Eos         = false;
	}


	bool VideoClip::Load(const char* path)
	{
		if (!path || path[0] == '\0') return false;

		// パスの末尾が「/」の場合はフォルダなのでコマ撮りで処理する
		std::string p = path;
		if (p.back() == '/' || p.back() == '\\')
		{
			return LoadFrameSequence(p);
		}
		// 動画の拡張子の場合は動画として処理する
		const std::string ext = GetLowerExtension(p);
		if (ext == ".mp4" || ext == ".wmv" || ext == ".avi" || ext == ".mov")
		{
			return LoadMP4(p);
		}
		// それ以外はフォルダとみなしてコマ撮りで処理する
		return LoadFrameSequence(p + "/");
	}


	bool VideoClip::LoadFrameSequence(const std::string& folderPath)
	{
		// 相対パス → 絶対パスに変換（LoadMP4 と同様）
		char absPathBuf[MAX_PATH] = {};
		GetFullPathNameA(folderPath.c_str(), MAX_PATH, absPathBuf, nullptr);
		std::string absFolder = absPathBuf;

		// GetFileAttributesA はトレイリングスラッシュがあると失敗する場合があるので除去してチェック
		std::string checkPath = absFolder;
		if (!checkPath.empty() && (checkPath.back() == '/' || checkPath.back() == '\\'))
			checkPath.pop_back();

		K2_LOG("VideoClip::LoadFrameSequence: checking %s\n", checkPath.c_str());
		const DWORD attr = GetFileAttributesA(checkPath.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			K2_LOG("VideoClip: folder not found: %s\n", checkPath.c_str());
			return false;
		}

		// CollectImageFiles にはトレイリングスラッシュ付きで渡す（ファイル名連結用）
		if (absFolder.empty() || (absFolder.back() != '/' && absFolder.back() != '\\'))
			absFolder += '\\';

		// フォルダ内の画像ファイルを収集
		const std::vector<std::string> files = CollectImageFiles(absFolder);
		K2_LOG("VideoClip::LoadFrameSequence: found %d image file(s)\n", static_cast<int>(files.size()));
		if (files.empty())
		{
			K2_LOG("VideoClip: no image files in: %s\n", folderPath.c_str());
			return false;
		}
		// 最初の画像からサイズを取得（以降は全て同サイズであることを期待）
		{
			int w = 0, h = 0, ch = 0;
			unsigned char* tmp = beast_stbi_load(files[0].c_str(), &w, &h, &ch, 4);
			if (!tmp) {
				K2_LOG("VideoClip: failed to load: %s\n", files[0].c_str());
				return false;
			}
			beast_stbi_free(tmp);
			m_width = w;
			m_height = h;
		}
		// 1フレームあたりのバイト数（RGBA32 なので幅×高さ×4）を計算して、全フレームをロード
		const int bytesPerFrame = m_width * m_height * 4;
		m_frames.clear();
		m_frames.reserve(files.size());

		for (const auto& fp : files)
		{
			int fw = 0, fh = 0, fch = 0;
			unsigned char* pixels = beast_stbi_load(fp.c_str(), &fw, &fh, &fch, 4);
			// 読み込み失敗したファイルはスキップする
			if (!pixels) { K2_LOG("VideoClip: skip: %s\n", fp.c_str()); continue; }
			// サイズが異なるファイルはスキップする
			if (fw != m_width || fh != m_height)
			{
				beast_stbi_free(pixels);
				K2_LOG("VideoClip: size mismatch skip: %s\n", fp.c_str());
				continue;
			}
			m_frames.emplace_back(pixels, pixels + bytesPerFrame);
			beast_stbi_free(pixels);
		}

		if (m_frames.empty()) return false;
		// フレーム数はロードできたフレームの数
		m_frameCount = static_cast<int>(m_frames.size());
		// タイプはコマ撮り
		m_clipType = ClipType::FrameSequence;
		return true;
	}


	bool VideoClip::LoadMP4(const std::string& filePath)
	{
		// フレーム数が取得できなかった場合のフォールバック値（EndOfStream で停止する）
		constexpr int k_fallbackFrameCount = 100000;

		// Media Foundation を初期化してビデオファイルを開く
		HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		m_coInitialized = SUCCEEDED(hrCo);
		// CoInitializeEx に失敗しても MFStartup を試みる
		HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
		if (FAILED(hr))
		{
			K2_LOG("VideoClip: MFStartup failed hr=0x%08X\n", hr);
			if (m_coInitialized)
			{
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		// 相対パス → 絶対パス（MFCreateSourceReaderFromURL は絶対パスが必要）
		char absPathBuf[MAX_PATH] = {};
		GetFullPathNameA(filePath.c_str(), MAX_PATH, absPathBuf, nullptr);
		K2_LOG("VideoClip: opening %s\n", absPathBuf);

		const int wlen = MultiByteToWideChar(CP_ACP, 0, absPathBuf, -1, nullptr, 0);
		std::vector<wchar_t> wpath(wlen);
		MultiByteToWideChar(CP_ACP, 0, absPathBuf, -1, wpath.data(), wlen);

		// MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING を有効にすることで
		// H.264(NV12) → ARGB32 への変換 MFT が自動挿入される
		ComPtr<IMFAttributes> readerAttrs;
		MFCreateAttributes(&readerAttrs, 1);
		readerAttrs->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

		ComPtr<IMFSourceReader> reader;
		hr = MFCreateSourceReaderFromURL(wpath.data(), readerAttrs.Get(), &reader);
		if (FAILED(hr))
		{
			K2_LOG("VideoClip: cannot open: %s hr=0x%08X\n", filePath.c_str(), hr);
			MFShutdown();
			if (m_coInitialized)
			{
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		// 映像ストリームのみ選択
		reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
		reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

		// ARGB32 → RGB32 の順で試行（環境によって対応フォーマットが異なる）
		static const GUID kTryFormats[] = { MFVideoFormat_ARGB32, MFVideoFormat_RGB32 };
		bool fmtOk = false;
		for (int fi = 0; fi < _countof(kTryFormats) && !fmtOk; ++fi)
		{
			ComPtr<IMFMediaType> outType;
			MFCreateMediaType(&outType);
			outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			outType->SetGUID(MF_MT_SUBTYPE, kTryFormats[fi]);
			hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, outType.Get());
			K2_LOG("VideoClip: SetCurrentMediaType[%d] hr=0x%08X\n", fi, hr);
			if (SUCCEEDED(hr)) fmtOk = true;
		}
		if (!fmtOk)
		{
			K2_LOG("VideoClip: no usable output format\n");
			MFShutdown();
			if (m_coInitialized)
			{
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		// メタデータ取得
		ComPtr<IMFMediaType> actualType;
		reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actualType);

		// 実際に交渉された出力フォーマットを確認（ARGB32/RGB32 以外だと色化けする）
		{
			GUID subtypeActual = GUID_NULL;
			actualType->GetGUID(MF_MT_SUBTYPE, &subtypeActual);
			const bool isARGB32 = (subtypeActual == MFVideoFormat_ARGB32);
			const bool isRGB32  = (subtypeActual == MFVideoFormat_RGB32);
			const char* subtypeName = "";
			if	    (isARGB32) { subtypeName = "ARGB32"; }
			else if (isRGB32)  { subtypeName = "RGB32"; }
			else               { subtypeName = "OTHER (color may be wrong)"; }
			K2_LOG("VideoClip: actual output subtype = %s\n", subtypeName);
			if (!isARGB32 && !isRGB32)
			{
				K2_LOG("VideoClip: WARNING – unsupported output format. "
					   "Enable the Windows codec pack or re-encode the video as H.264 MP4.\n");
			}
		}

		UINT32 w = 0, h = 0;
		MFGetAttributeSize(actualType.Get(), MF_MT_FRAME_SIZE, &w, &h);
		m_width = static_cast<int>(w);
		m_height = static_cast<int>(h);

		UINT32 fpsNum = 0, fpsDen = 1;
		if (SUCCEEDED(MFGetAttributeRatio(actualType.Get(), MF_MT_FRAME_RATE, &fpsNum, &fpsDen))
			&& fpsDen > 0 && fpsNum > 0)
		{
			m_fps = static_cast<float>(fpsNum) / static_cast<float>(fpsDen);
		}

		// 再生時間 → フレーム数
		PROPVARIANT varDur;
		memset(&varDur, 0, sizeof(varDur));
		if (SUCCEEDED(reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &varDur))
			&& varDur.vt == VT_UI8)
		{
			const float durSec = static_cast<float>(varDur.uhVal.QuadPart) / 10000000.0f;
			m_frameCount = static_cast<int>(durSec * m_fps);
		}

		if (m_frameCount <= 0)
		{
			K2_LOG("VideoClip: duration not available, using fallback frameCount=%d\n", k_fallbackFrameCount);
			m_frameCount = k_fallbackFrameCount;
		}

		m_mp4FrameBuffer.resize(m_width * m_height * 4, 0);
		m_mfReader = reader.Detach(); // void* に所有権を移譲
		m_mp4CurrentFrame = -1;
		m_mp4Eos = false;
		m_clipType = ClipType::MP4;

		K2_LOG("VideoClip: MP4 ready %dx%d %.2ffps %dframes: %s\n",
			m_width, m_height, m_fps, m_frameCount, filePath.c_str());
		return true;
	}


	void VideoClip::CleanupMP4()
	{
		if (m_mfReader)
		{
			static_cast<IMFSourceReader*>(m_mfReader)->Release();
			m_mfReader = nullptr;
			MFShutdown();
		}
		if (m_coInitialized)
		{
			CoUninitialize();
			m_coInitialized = false;
		}
	}


	bool VideoClip::ReadNextMP4Frame() const
	{
		auto* reader = static_cast<IMFSourceReader*>(m_mfReader);
		if (!reader || m_mp4Eos) return false;

		DWORD    streamIdx = 0, flags = 0;
		LONGLONG timestamp = 0;
		IMFSample* sample = nullptr;

		HRESULT hr = reader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0, 
			&streamIdx,
			&flags, 
			&timestamp,
			&sample
		);

		if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM))
		{
			m_mp4Eos = true;
			if (sample) sample->Release();
			return false;
		}
		if (!sample) return false;

		IMFMediaBuffer* buf = nullptr;
		sample->ConvertToContiguousBuffer(&buf);
		sample->Release();
		if (!buf) return false;

		// IMF2DBuffer でストライドを考慮しつつ BGRA → RGBA 変換
		IMF2DBuffer* buf2D = nullptr;
		if (SUCCEEDED(buf->QueryInterface(IID_PPV_ARGS(&buf2D))))
		{
			BYTE* scan0 = nullptr;
			LONG  pitch = 0;
			if (SUCCEEDED(buf2D->Lock2D(&scan0, &pitch)))
			{
				for (int y = 0; y < m_height; ++y)
				{
					const BYTE* row = scan0 + static_cast<ptrdiff_t>(y) * pitch;
					const int   dstY = (pitch >= 0) ? y : (m_height - 1 - y);
					uint8_t* dst = m_mp4FrameBuffer.data() + dstY * m_width * 4;
					for (int x = 0; x < m_width; ++x)
					{
						dst[x * 4 + 0] = row[x * 4 + 2]; // R (BGRx → RGBA)
						dst[x * 4 + 1] = row[x * 4 + 1]; // G
						dst[x * 4 + 2] = row[x * 4 + 0]; // B
						dst[x * 4 + 3] = 0xFF;           // A (RGB32 は X=0 なので強制不透明)
					}
				}
				buf2D->Unlock2D();
			}
			buf2D->Release();
		}
		else
		{
			// 1D フォールバック
			// H.264 は高さを 16 の倍数に揃えるため curLen が m_height より多い行分を含む場合がある。
			// curLen / m_height で計算すると行ストライドが狂うため、水平パディングのない
			// MF ARGB32 出力を前提に m_width * 4 を使用する。
			BYTE* data = nullptr;
			DWORD maxLen = 0, curLen = 0;
			if (SUCCEEDED(buf->Lock(&data, &maxLen, &curLen)))
			{
				const int stride = m_width * 4;
				for (int y = 0; y < m_height; ++y)
				{
					const BYTE* row = data + y * stride;
					uint8_t* dst = m_mp4FrameBuffer.data() + y * m_width * 4;
					for (int x = 0; x < m_width; ++x)
					{
						dst[x * 4 + 0] = row[x * 4 + 2];
						dst[x * 4 + 1] = row[x * 4 + 1];
						dst[x * 4 + 2] = row[x * 4 + 0];
						dst[x * 4 + 3] = 0xFF;
					}
				}
				buf->Unlock();
			}
		}

		buf->Release();

		m_mp4CurrentFrame++;
		return true;
	}


	bool VideoClip::SeekMP4ToBeginning() const
	{
		auto* reader = static_cast<IMFSourceReader*>(m_mfReader);
		if (!reader) return false;

		PROPVARIANT varPos;
		memset(&varPos, 0, sizeof(varPos));
		varPos.vt = VT_I8;
		varPos.hVal.QuadPart = 0LL;

		const bool ok = SUCCEEDED(reader->SetCurrentPosition(GUID_NULL, varPos));
		if (ok) {
			m_mp4CurrentFrame = -1;
			m_mp4Eos = false;
		}
		return ok;
	}


	const uint8_t* VideoClip::GetFramePixels(int frameIndex) const
	{
		if (m_clipType == ClipType::FrameSequence)
		{
			if (frameIndex < 0 || frameIndex >= static_cast<int>(m_frames.size())) {
				return nullptr;
			}
			return m_frames[frameIndex].data();
		}

		if (m_clipType == ClipType::MP4)
		{
			if (!m_mfReader) return nullptr;

			// ループ巻き戻し
			if (frameIndex < m_mp4CurrentFrame) {
				SeekMP4ToBeginning();
			}

			if (frameIndex == m_mp4CurrentFrame) {
				return m_mp4FrameBuffer.data();
			}

			while (m_mp4CurrentFrame < frameIndex && !m_mp4Eos) {
				ReadNextMP4Frame();
			}

			return m_mp4FrameBuffer.empty() ? nullptr : m_mp4FrameBuffer.data();
		}

		return nullptr;
	}
}
