/**
 * @file Achievement.cpp
 * @brief アチーブメントに関するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementBase.h"
#include "Source/Util/CRC32.h"


namespace
{
	/**
	 * @brief jsonからVector3をパースする関数
	 */
	Vector3 ParseVector3(const nlohmann::json& arr)
	{
		return Vector3(
			arr[0].get<float>(),
			arr[1].get<float>(),
			arr[2].get<float>()
		);
	}
}


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
			m_name = json["name"].get<std::string>();
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
			m_targetValue = json["targetValue"].get<uint32_t>();
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
			m_targetLocation = ParseVector3(json["targetLocation"]);
			m_enableDistance = json["thresholdDistance"].get<float>();
		}
	}
}