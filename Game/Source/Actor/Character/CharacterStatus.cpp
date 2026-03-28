/**
 * @file CharacterStatus.cpp
 * @brief キャラクターのステータス基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterStatus.h"


namespace app
{
	namespace actor
	{
		CharacterStatus::CharacterStatus()
			: m_runSpeed(0.0f)
			, m_radius(0.0f)
			, m_height(1.0f)
		{}


		void CharacterStatus::Setup()
		{}


		void CharacterStatus::Update()
		{}
	}
}