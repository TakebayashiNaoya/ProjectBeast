/**
 * @file LogCompression.cpp
 * @brief プレイログファイルの圧縮・展開ユーティリティ
 */
#include "stdafx.h"
#include "LogCompression.h"

// ZIPアーカイブ機能（miniz_zip.c/.h）は使わないため、コンパイル時に無効化する
// （圧縮・展開そのものはminiz.c内のzlib互換API mz_compress2/mz_uncompressが提供する）
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#include "miniz.h"

#include <cstring>


namespace app
{
	namespace
	{
		/** 圧縮レベル（0=無圧縮〜10=最高圧縮。MZ_DEFAULT_COMPRESSIONはzlib準拠のバランス値） */
		constexpr int LOG_COMPRESS_LEVEL = MZ_DEFAULT_COMPRESSION;

		/**
		 * @brief 圧縮データの先頭に埋め込む展開後サイズのバイト数
		 * @details mz_uncompress() は展開先バッファを呼び出し側が事前に確保する必要があり、
		 *          zlib形式のストリーム自体には元のサイズが含まれていないため、
		 *          自前で先頭に4バイト（リトルエンディアン、uint32_t）として埋め込んでおく
		 */
		constexpr size_t HEADER_SIZE = sizeof(uint32_t);
	}


	std::vector<uint8_t> CompressLogData(const std::string& data)
	{
		if (data.empty()) return {};
		if (data.size() > 0xFFFFFFFFu) return {}; // 4バイトヘッダーで表現できる上限を超える

		mz_ulong compressedBound = mz_compressBound(static_cast<mz_ulong>(data.size()));
		std::vector<uint8_t> buffer(compressedBound);

		mz_ulong compressedSize = compressedBound;
		const int result = mz_compress2(
			buffer.data(), &compressedSize,
			reinterpret_cast<const unsigned char*>(data.data()), static_cast<mz_ulong>(data.size()),
			LOG_COMPRESS_LEVEL);

		if (result != MZ_OK) return {};

		std::vector<uint8_t> out(HEADER_SIZE + compressedSize);
		const uint32_t originalSize = static_cast<uint32_t>(data.size());
		std::memcpy(out.data(), &originalSize, HEADER_SIZE);
		std::memcpy(out.data() + HEADER_SIZE, buffer.data(), compressedSize);
		return out;
	}


	std::string DecompressLogData(const std::vector<uint8_t>& compressed)
	{
		if (compressed.size() <= HEADER_SIZE) return {};

		uint32_t originalSize = 0;
		std::memcpy(&originalSize, compressed.data(), HEADER_SIZE);

		std::string decompressed(originalSize, '\0');
		mz_ulong destLen = originalSize;

		const int result = mz_uncompress(
			reinterpret_cast<unsigned char*>(decompressed.data()), &destLen,
			compressed.data() + HEADER_SIZE, static_cast<mz_ulong>(compressed.size() - HEADER_SIZE));

		if (result != MZ_OK) return {};

		decompressed.resize(destLen);
		return decompressed;
	}
}
