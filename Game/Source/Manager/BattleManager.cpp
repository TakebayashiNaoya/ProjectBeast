/**
 * @file BattleManager.cpp
 * @brief バトルの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "BattleManager.h"


namespace app
{
	BattleManager* BattleManager::m_instance = nullptr;

	BattleManager::BattleManager()
		: m_isGameActive(false)
		, m_isClear(false)
	{}

	BattleManager::~BattleManager()
	{}
}