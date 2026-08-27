/**
 * @file ChildPenguinTypes.h
 * @brief 子ペンギンの型定義
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
			Cluster,		// 密集陣（防御特化）
			Scatter,		// 散開陣（収集特化）
			Num
		};


		/**
		 * @brief 陣形の名前を返す
		 * @param type 陣形の種類
		 * @return 陣形の名前
		 * @note プレイログに陣形の切り替えやウルトを残すために使う
		 */
		inline const char* FormationTypeName(EnFormationType type)
		{
			switch (type)
			{
			case EnFormationType::Circle:   return "Circle";
			case EnFormationType::Triangle: return "Triangle";
			case EnFormationType::Cluster:  return "Cluster";
			case EnFormationType::Scatter:  return "Scatter";
			default:                        return "Unknown";
			}
		}
	}
}