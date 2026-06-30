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
			Naughty,		// やんちゃ
			Clumsy,			// おっちょこちょい
			Caring,			// 世話焼き
			Num
		};


		/**
		 * @brief 陣形の種類
		 */
		enum class EnFormationType : uint8_t
		{
			Circle   = 0,	// 円陣（通常）
			Triangle,		// 三角陣（スピード特化）
			Defense,		// 密集陣（防御特化）
			Scatter,		// 散開陣（収集特化）
			Num
		};
	}
}