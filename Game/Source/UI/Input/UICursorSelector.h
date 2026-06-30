/**
 * @file UICursorSelector.h
 * @brief UIカーソル選択を行うクラス
 * @author 藤谷
 */
#pragma once
#include "Types.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief カーソルのインデックスを選択するクラス
		 * @details
		 * - 入力方向に応じてインデックスを動かすことができる
		 * - インデックスが動いた場合はSEを鳴らす
		 */
		class CursorIndexSelector
		{
		public:
			// dirがNoneでなければインデックスを動かしてSEを鳴らす
			bool TryMove(Direction dir);


			inline int Get() const { return m_index; }
			inline void Set(int v) { m_index = v; }

			inline void Reset() { m_index = 0; }


			CursorIndexSelector(int max);
			~CursorIndexSelector() = default;


		private:
			int m_index;
			int m_max;
		};
	}
}


