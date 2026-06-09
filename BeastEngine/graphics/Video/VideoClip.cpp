/**
 * @file VideoClip.cpp
 * @brief コマ撮り／映像フレームデータ管理クラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "VideoClip.h"
#include <algorithm>
#include <cctype>

 // stb_image ラッパー関数の宣言（stb_image_impl.cpp で定義）
unsigned char* beast_stbi_load(const char* filename, int* x, int* y, int* channels, int desired);
void           beast_stbi_free(void* ptr);


namespace nsBeastEngine
{
	// -------------------------------------------------------
	// ファイルパスから拡張子を小文字で取り出すヘルパー
	// -------------------------------------------------------
	static std::string GetLowerExtension(const std::string& path)
	{
		const auto dot = path.rfind('.');
		if (dot == std::string::npos) return "";
		std::string ext = path.substr(dot);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext;
	}


	// -------------------------------------------------------
	// フォルダ内の画像ファイルを Windows API で収集しソートする
	// std::filesystem を使わず <Windows.h>（BeastEnginePreCompile 経由）だけで完結
	// -------------------------------------------------------
	static std::vector<std::string> CollectImageFiles(const std::string& folderPath)
	{
		std::vector<std::string> files;

		// folderPath が末尾スラッシュ付きであることを前提に "*.* " を検索
		std::string searchPath = folderPath + "*";

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

		// ファイル名順でソート（0000.png → 0001.png → ...）
		std::sort(files.begin(), files.end());
		return files;
	}


	// -------------------------------------------------------
	// VideoClip
	// -------------------------------------------------------

	bool VideoClip::Load(const char* path)
	{
		if (!path || path[0] == '\0') return false;

		std::string p = path;

		// 末尾が / or \ → コマ撮りフォルダ
		if (p.back() == '/' || p.back() == '\\') {
			return LoadFrameSequence(p);
		}

		// 拡張子で判定
		const std::string ext = GetLowerExtension(p);

		if (ext == ".mp4" || ext == ".wmv" || ext == ".avi")
		{
			K2_LOG("VideoClip::Load: MP4/WMV/AVI は未対応です。フォルダパスを使用してください。\n");
			return false;
		}

		// 不明な場合はフォルダとして試みる
		return LoadFrameSequence(p + "/");
	}


	bool VideoClip::LoadFrameSequence(const std::string& folderPath)
	{
		// フォルダの存在確認
		const DWORD attr = GetFileAttributesA(folderPath.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			K2_LOG("VideoClip: フォルダが見つかりません: %s\n", folderPath.c_str());
			return false;
		}

		const std::vector<std::string> files = CollectImageFiles(folderPath);
		if (files.empty())
		{
			K2_LOG("VideoClip: 画像ファイルが見つかりません: %s\n", folderPath.c_str());
			return false;
		}

		// 最初のフレームで解像度を確定
		{
			int w = 0, h = 0, ch = 0;
			unsigned char* tmp = beast_stbi_load(files[0].c_str(), &w, &h, &ch, 4);
			if (!tmp)
			{
				K2_LOG("VideoClip: フレームの読み込みに失敗しました: %s\n", files[0].c_str());
				return false;
			}
			beast_stbi_free(tmp);
			m_width = w;
			m_height = h;
		}

		// 全フレームを読み込み
		const int bytesPerFrame = m_width * m_height * 4;
		m_frames.clear();
		m_frames.reserve(files.size());

		for (const auto& fp : files)
		{
			int fw = 0, fh = 0, fch = 0;
			unsigned char* pixels = beast_stbi_load(fp.c_str(), &fw, &fh, &fch, 4);
			if (!pixels)
			{
				K2_LOG("VideoClip: フレーム読み込み失敗（スキップ）: %s\n", fp.c_str());
				continue;
			}
			if (fw != m_width || fh != m_height)
			{
				K2_LOG("VideoClip: フレームサイズ不一致（スキップ）: %s\n", fp.c_str());
				beast_stbi_free(pixels);
				continue;
			}

			m_frames.emplace_back(pixels, pixels + bytesPerFrame);
			beast_stbi_free(pixels);
		}

		return !m_frames.empty();
	}


	const uint8_t* VideoClip::GetFramePixels(int frameIndex) const
	{
		if (frameIndex < 0 || frameIndex >= static_cast<int>(m_frames.size())) {
			return nullptr;
		}
		return m_frames[frameIndex].data();
	}
}
