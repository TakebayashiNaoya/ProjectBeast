/**
 * @file MiniMapTypes.h
 * @brief ミニマップの型定義
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace ui
	{
		/**
		 * @brief ミニマップのアイコンの種類
		 * @detail
		 *	まじめ、
		 *	甘えん坊、
		 *	やんちゃ、
		 *	おっちょこちょい、
		 *	世話焼き、
		 *	シロクマの巣、
		 *	シロクマ、
		 *	渦潮、
		 *	イグルー
		 */
		enum class EnMiniMapIconType : uint8_t
		{
			Serious = 0,
			Clingy,
			Naughty,
			Clumsy,
			Caring,
			BearNest,
			Bear,
			Whirlpool,
			Igloo,
			Num
		};
	}
}