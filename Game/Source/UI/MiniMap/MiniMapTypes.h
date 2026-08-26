/**
 * @file MiniMapTypes.h
 * @brief ミニマップの型定義
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


		/**
		 * @brief ミニマップアイコンの初期化情報
		 */
		struct MiniMapIconInitializeInfo
		{
			/** アセットパス */
			std::string path;
			/** 幅 */
			float width;
			/** 高さ */
			float height;
		};


		using MiniMapInitializeInfo = std::array<MiniMapIconInitializeInfo, static_cast<uint8_t>(EnMiniMapIconType::Num)>;


		using ActorPositions = std::array<std::vector<Vector3>, static_cast<uint8_t>(ui::EnMiniMapIconType::Num)>;
	}
}