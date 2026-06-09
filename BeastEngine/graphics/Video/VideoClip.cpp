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


	bool VideoClip::Load(const char* path)
	{
		if (!path || path[0] == '\0') return false;

		// パスの末尾が「/」の場合はフォルダなのでコマ撮りで処理する
		std::string p = path;
		if (p.back() == '/' || p.back() == '\\') {
			return LoadFrameSequence(p);
		}
		// 動画の拡張子の場合は動画として処理する
		const std::string ext = GetLowerExtension(p);
		if (ext == ".mp4" || ext == ".wmv" || ext == ".avi" || ext == ".mov") {
			return LoadMP4(p);
		}
		// それ以外はフォルダとみなしてコマ撮りで処理する
		return LoadFrameSequence(p + "/");
	}


	bool VideoClip::LoadFrameSequence(const std::string& folderPath)
	{
		// フォルダの存在確認
		const DWORD attr = GetFileAttributesA(folderPath.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			K2_LOG("VideoClip: folder not found: %s\n", folderPath.c_str());
			return false;
		}
		// フォルダ内の画像ファイルを収集
		const std::vector<std::string> files = CollectImageFiles(folderPath);
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
		// Media Foundation を初期化してビデオファイルを開く
		HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		m_coInitialized = SUCCEEDED(hrCo);
		// CoInitializeEx に失敗しても MFStartup を試みる
		HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
		if (FAILED(hr))
		{
			K2_LOG("VideoClip: MFStartup failed hr=0x%08X\n", hr);
			if (m_coInitialized) {
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		const int wlen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
		std::vector<wchar_t> wpath(wlen);
		MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, wpath.data(), wlen);

		IMFSourceReader* reader = nullptr;
		hr = MFCreateSourceReaderFromURL(wpath.data(), nullptr, &reader);
		if (FAILED(hr))
		{
			K2_LOG("VideoClip: cannot open: %s hr=0x%08X\n", filePath.c_str(), hr);
			MFShutdown();
			if (m_coInitialized) {
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		// 出力フォーマットを ARGB32（メモリ上は BGRA）に設定
		IMFMediaType* outType = nullptr;
		MFCreateMediaType(&outType);
		outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		outType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
		hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, outType);
		outType->Release();
		if (FAILED(hr))
		{
			K2_LOG("VideoClip: SetCurrentMediaType failed hr=0x%08X\n", hr);
			reader->Release(); MFShutdown();
			if (m_coInitialized) {
				CoUninitialize();
				m_coInitialized = false;
			}
			return false;
		}

		// メタデータ取得
		IMFMediaType* actualType = nullptr;
		reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actualType);

		UINT32 w = 0, h = 0;
		MFGetAttributeSize(actualType, MF_MT_FRAME_SIZE, &w, &h);
		m_width = static_cast<int>(w);
		m_height = static_cast<int>(h);

		UINT32 fpsNum = 0, fpsDen = 1;
		if (SUCCEEDED(MFGetAttributeRatio(actualType, MF_MT_FRAME_RATE, &fpsNum, &fpsDen))
			&& fpsDen > 0 && fpsNum > 0)
		{
			m_fps = static_cast<float>(fpsNum) / static_cast<float>(fpsDen);
		}
		actualType->Release();

		// 再生時間 → フレーム数
		PROPVARIANT varDur;
		memset(&varDur, 0, sizeof(varDur));
		if (SUCCEEDED(reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &varDur))
			&& varDur.vt == VT_UI8)
		{
			const float durSec = static_cast<float>(varDur.uhVal.QuadPart) / 10000000.0f;
			m_frameCount = static_cast<int>(durSec * m_fps);
		}

		m_mp4FrameBuffer.resize(m_width * m_height * 4, 0);
		m_mfReader = reader;
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
			0, &streamIdx, &flags, &timestamp, &sample);

		if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM))
		{
			m_mp4Eos = true;
			if (sample) {
				sample->Release();
			}
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
						dst[x * 4 + 0] = row[x * 4 + 2]; // R
						dst[x * 4 + 1] = row[x * 4 + 1]; // G
						dst[x * 4 + 2] = row[x * 4 + 0]; // B
						dst[x * 4 + 3] = row[x * 4 + 3]; // A
					}
				}
				buf2D->Unlock2D();
			}
			buf2D->Release();
		}
		else
		{
			// 1D フォールバック（ストライドをバッファサイズから推定）
			BYTE* data = nullptr;
			DWORD maxLen = 0, curLen = 0;
			if (SUCCEEDED(buf->Lock(&data, &maxLen, &curLen)))
			{
				const int stride = (m_height > 0)
					? static_cast<int>(curLen / m_height)
					: m_width * 4;
				for (int y = 0; y < m_height; ++y)
				{
					const BYTE* row = data + y * stride;
					uint8_t* dst = m_mp4FrameBuffer.data() + y * m_width * 4;
					for (int x = 0; x < m_width; ++x)
					{
						dst[x * 4 + 0] = row[x * 4 + 2];
						dst[x * 4 + 1] = row[x * 4 + 1];
						dst[x * 4 + 2] = row[x * 4 + 0];
						dst[x * 4 + 3] = row[x * 4 + 3];
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
