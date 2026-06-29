/**
 * @file TimeManager.cpp
 * @brief タイムの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "TimeManager.h"


namespace app
{
	TimeManager* TimeManager::m_instance = nullptr;


	void TimeManager::Update()
	{
		if (m_isTimeStop) return;

		LONGLONG now;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&now));

		if (m_lastUpdateTime != 0)
		{
			const float delta = static_cast<float>(
				static_cast<double>(now - m_lastUpdateTime) / static_cast<double>(m_freq)
			);
			m_currentTime -= delta;
		}
		m_lastUpdateTime = now;

		if (m_currentTime <= 0.0f)
		{
			m_currentTime = 0.0f;
			m_isTimeStop = true;
			m_isTimeUp = true;

			BattleManager::GetInstance().SetTimeUp(m_isTimeUp);
		}

		BattleManager::GetInstance().SetCurrentTime(m_currentTime);
	}


	void TimeManager::ResetTime()
	{
		m_currentTime = m_maxTime;
		m_isTimeStop = false;
		m_isTimeUp = false;
		m_lastUpdateTime = 0;
	}
}