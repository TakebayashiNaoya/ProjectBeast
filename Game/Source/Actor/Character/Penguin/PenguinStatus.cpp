/**
 * @file PenguinStatus.cpp
 * @brief ペンギンのステータスクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinStatus.h"


namespace app
{
	namespace actor
	{
		void PenguinStatus::Setup()
		{}


		void PenguinStatus::Update()
		{}


		PenguinStatus::PenguinStatus()
			: m_maxHp(0)
			, m_hp(0)
			, m_sneakSpeed(0.0f)
			, m_slideSpeed(0.0f)
			, m_jumpPower(0.0f)
			, m_jumpStaminaMax(0.0f)
			, m_jumpStaminaRecoverSpeed(0.0f)
			, m_slideStaminaMax(0.0f)
			, m_slideStaminaDecreaseSpeed(0.0f)
			, m_slideStaminaRecoverSpeed(0.0f)
		{}


		PenguinStatus::~PenguinStatus()
		{}
	}
}