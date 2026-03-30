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

	TimeManager::TimeManager()
		:m_maxTime(0.0f)
		, m_currentTime(300.0f)
	{

	}

	TimeManager::~TimeManager()
	{

	}


	void TimeManager::Update()
	{
		if (m_timeStop) return;

		m_currentTime -= g_gameTime->GetFrameDeltaTime();
		if (m_currentTime <= m_maxTime)
		{
			m_currentTime = 0.0f;
		}
	}
}