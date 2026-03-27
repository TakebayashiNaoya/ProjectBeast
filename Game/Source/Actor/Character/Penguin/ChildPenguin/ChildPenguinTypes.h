/**
 * @file ChildPenguinTypes.h
 * @brief 子ペンギンの型定義
 * @author	竹林
 */
#pragma once
#include <cstdint>

namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンのタイプ
		 */
		enum class EnChildPenguinType : uint8_t
		{
			Serious = 0,	// まじめ
			Clingy,			// 甘えん坊
			naughty,		// やんちゃ
			Clumsy,			// おっちょこちょい
			Caring,			// 世話焼き
			Num
		};
	}
}