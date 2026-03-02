/**
 * @file Achievement.cpp
 * @brief アチーブメントに関するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementBase.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace achievement
	{
		void AchievementBase::Init(const nlohmann::json& json)
		{
			InitAchievementBase(json);
			InitAchievementImpl(json);
		}


		void AchievementBase::InitAchievementBase(const nlohmann::json& json)
		{
			m_name = app::util::JsonConverter::ToString(json["name"]);
			m_name = app::util::JsonConverter::ToString(json["condition"]);
			m_id = Hash32(m_name.c_str());
		}


		AchievementBase::AchievementBase()
			: m_name("")
			, m_description("")
			, m_id(0)
			, m_isAchieved(false)
			, m_achievedTime(0)
		{
			m_flags.clear();
		}


		AchievementBase::~AchievementBase()
		{
		}




		/*************************************************/


		void CounterAchievement::Update()
		{

		}


		CounterAchievement::CounterAchievement()
			: m_currentValue(0)
			, m_targetValue(0)
		{
		}


		CounterAchievement::~CounterAchievement()
		{
		}


		void CounterAchievement::InitAchievementImpl(const nlohmann::json& json)
		{
			m_targetValue = app::util::JsonConverter::ToUInt32(json["targetValue"]);
		}




		/*************************************************/


		LocationAchievement::LocationAchievement()
			: m_targetLocation(Vector3::Zero)
			, m_enableDistance(0.0f)
		{
		}


		LocationAchievement::~LocationAchievement()
		{
		}


		void LocationAchievement::InitAchievementImpl(const nlohmann::json& json)
		{
			m_targetLocation = app::util::JsonConverter::ToVector3(json["targetLocation"]);
			m_enableDistance = app::util::JsonConverter::ToFloat(json["enableDistance"]);
		}
	}
}