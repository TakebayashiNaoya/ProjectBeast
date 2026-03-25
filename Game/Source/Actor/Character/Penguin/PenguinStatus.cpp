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
			: m_sneakSpeed(0.0f)
			, m_slideSpeed(0.0f)
			, m_jumpPower(1.0f)
		{}


		PenguinStatus::~PenguinStatus()
		{}
	}
}