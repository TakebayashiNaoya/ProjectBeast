/**
 * @file PlayerStatus.cpp
 * @brief プレイヤーのステータスクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "PlayerStatus.h"


namespace app
{
	namespace actor
	{
		void PlayerStatus::Setup()
		{
			// プレイヤー固有のステータスを設定
			m_maxHp = 150;
			m_hp = m_maxHp;
			m_walkSpeed = 0.05f;
			m_runSpeed = 0.1f;
			m_radius = 0.5f;
			m_height = 2.0f;
		}
	}
}