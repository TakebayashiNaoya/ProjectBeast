/**
 * @file LogCompression.h
 * @brief プレイログファイルの圧縮・展開ユーティリティ
 * @author 竹林
 * @details
 * Third Party/miniz（zlib互換のpublic domainライブラリ）を使ってzlib形式で圧縮する。
 * Pythonの zlib モジュール等、zlib形式を扱えるツールであれば読み書きできる
 * （ただし本プロジェクトの.cmpファイルは先頭4バイトに展開後サイズを独自に
 * 埋め込んでいるため、その部分だけは本プロジェクト独自）。
 */
#pragma once
#include <string>
#include <vector>
#include <cstdint>


namespace app
{
	/**
	 * @brief 文字列を圧縮する
	 * @param data 圧縮したい元データ
	 * @return 圧縮後のバイト列（失敗時は空）
	 */
	std::vector<uint8_t> CompressLogData(const std::string& data);

	/**
	 * @brief CompressLogData() で圧縮したバイト列を元の文字列へ展開する
	 * @param compressed 圧縮済みバイト列
	 * @return 展開後の文字列（失敗時は空）
	 */
	std::string DecompressLogData(const std::vector<uint8_t>& compressed);
}
