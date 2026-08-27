/**
 * @file UIInputController.h
 * @brief UI入力制御を行うクラス
 */
#pragma once


namespace app
{
	namespace ui
	{

		/**
		 * @brief 入力方向の列挙型
		 * @details
		 * - None: 入力なし
		 * - Negative: 負の方向に入力
		 * - Positive: 正の方向に入力
		 */
		enum class Direction : uint8_t
		{
			None,
			Negative,
			Positive
		};
	}
}


