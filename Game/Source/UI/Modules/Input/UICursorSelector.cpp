/**
 * @file UICursorSelector.cpp
 * @brief UIカーソル選択を行うクラス
 */
#include "stdafx.h"
#include "UICursorSelector.h"

#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		bool CursorIndexSelector::TryMove(Direction dir)
		{
			if (dir == Direction::None) return false;

			const int delta = (dir == Direction::Positive) ? 1 : -1;
			m_index = (m_index + delta + m_max) % m_max;

			SoundManager::Get().PlaySE(static_cast<int>(enSoundKind::enSoundKind_CursorMove));
			return true;
		}


		CursorIndexSelector::CursorIndexSelector(const int max)
			: m_index(0)
			, m_max(max)
		{}
	}
}