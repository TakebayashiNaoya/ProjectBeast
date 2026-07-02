/**
 * @file BearReactionTypes.h
 * @brief クマのリアクションタイプ定義
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace ui
	{
		/**
		 * @brief リアクションタイプ
		 * @details Tongue:舌、Debuff:デバフ、Bed:ベッド、None:なし
		 */
		enum class EnBearReactionType : uint8_t
		{
			Tongue,
			Debuff,
			Bed,
			None,
			Max
		};
	}
}


