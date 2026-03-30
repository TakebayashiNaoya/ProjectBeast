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

		m_currentTime -= g_gameTime->GetFrameDeltaTime();

		if (m_currentTime <= m_maxTime)
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
	}
}