// stb_image 実装ファイル
// PCH を使わず単体でコンパイルする（Windows.h との競合を避けるため）
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// VideoClip.cpp から呼び出す C++ ラッパー（extern "C" を使わず通常 C++ リンケージ）
unsigned char* beast_stbi_load(const char* filename, int* x, int* y, int* channels, int desired)
{
	return stbi_load(filename, x, y, channels, desired);
}

void beast_stbi_free(void* ptr)
{
	stbi_image_free(ptr);
}
