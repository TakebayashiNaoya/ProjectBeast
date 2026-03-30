/**
 * @file ScoreManager.cpp
 * @brief スコアの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "ScoreManager.h"
#include <algorithm>


namespace app
{
	ScoreManager* ScoreManager::m_instance = nullptr;

	ScoreManager::ScoreManager()
		:m_collectedCount(3)
	{

	}


	ScoreManager::~ScoreManager()
	{

	}
}