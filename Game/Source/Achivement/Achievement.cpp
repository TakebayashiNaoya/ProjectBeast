/**
 * @file Achievement.cpp
 * @brief アチーブメントに関するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "Achievement.h"


namespace app
{
	namespace achievement
	{
		void Achievement::InitializeAchievement(const std::string& name, const std::string& description, uint32_t id, std::function<bool()> conditionFunc)
		{
			m_name = name;
			m_description = description;
			m_id = id;
			m_conditionFunc = conditionFunc;
		}


		void Achievement::Update()
		{
			m_isAchieved = m_conditionFunc();
		}


		Achievement::Achievement()
			: m_name("")
			, m_description("")
			, m_id(0)
			, m_isAchieved(false)
			, m_achievedTime(0)
		{
		}


		Achievement::~Achievement()
		{
		}
	}
}