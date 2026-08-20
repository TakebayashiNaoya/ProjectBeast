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


		void AchievementBase::MarkAchieved()
		{
			if (m_isAchieved) return;

			m_isAchieved = true;

			// ログの "t" と揃えて残り秒数で持つ。経過秒数はステージの制限時間から引いて求める
			m_achievedTime = static_cast<uint32_t>(TimeManager::GetInstance().GetCurTime());
		}


		void AchievementBase::InitAchievementBase(const nlohmann::json& json)
		{
			m_name        = app::util::JsonConverter::ToString(json, "name");
			m_description = app::util::JsonConverter::ToString(json, "description");
			m_id          = Hash32(m_name.c_str());
			m_index       = app::util::JsonConverter::ToInt(json, "spriteIndex", -1);
			m_spriteName  = app::util::JsonConverter::ToString(json, "spriteName");
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
			m_flags.clear();
		}




		/*************************************************/


		void CounterAchievement::Update()
		{
			// 達成済みなら何もしない
			if (m_isAchieved) return;

			// 条件関数が設定されていれば評価して達成判定する
			if (m_conditionFunc && m_conditionFunc())
			{
				MarkAchieved();
			}
		}


		CounterAchievement::CounterAchievement()
			: m_currentValue(0)
			, m_targetValue(0)
		{}


		CounterAchievement::~CounterAchievement()
		{}


		void CounterAchievement::InitAchievementImpl(const nlohmann::json& json)
		{
			m_targetValue = app::util::JsonConverter::ToUInt32(json, "targetValue");
		}




		/*************************************************/


		LocationAchievement::LocationAchievement()
			: m_targetLocation(Vector3::Zero)
			, m_enableDistance(0.0f)
		{}


		LocationAchievement::~LocationAchievement()
		{}


		void LocationAchievement::Update()
		{

		}


		void LocationAchievement::InitAchievementImpl(const nlohmann::json& json)
		{
			m_targetLocation = app::util::JsonConverter::ToVector3(json, "targetLocation");
			m_enableDistance = app::util::JsonConverter::ToFloat(json, "enableDistance");
		}




		/*************************************************/


		ConditionAchievement::ConditionAchievement()
		{}


		ConditionAchievement::~ConditionAchievement()
		{}


		void ConditionAchievement::Update()
		{
			if (!m_isAchieved && m_conditionFunc && m_conditionFunc())
			{
				MarkAchieved();
			}
		}


		void ConditionAchievement::InitAchievementImpl(const nlohmann::json& json)
		{
			// Condition型はJSONから特有のパラメータを受け取らないため空でOK
		}




		/*************************************************/


		EventAchievement::EventAchievement()
		{}


		EventAchievement::~EventAchievement()
		{}


		void EventAchievement::Unlock()
		{
			if (!m_isAchieved)
			{
				MarkAchieved();
			}
		}




		/*************************************************/


		RecordAchievement::RecordAchievement()
			: m_recordValue(0)
		{}


		RecordAchievement::~RecordAchievement()
		{}


		void RecordAchievement::UpdateRecord(uint32_t value)
		{
			if (value > m_recordValue)
			{
				m_recordValue = value;
				// 記録が更新された時点で「達成」扱いにする（0以上の記録があるかどうかの判定用）
				MarkAchieved();
			}
		}




		/*************************************************/


		FinalConditionAchievement::FinalConditionAchievement()
		{}


		FinalConditionAchievement::~FinalConditionAchievement()
		{}


		void FinalConditionAchievement::Finalize()
		{
			if (!m_isAchieved && m_conditionFunc && m_conditionFunc())
			{
				MarkAchieved();
			}
		}
	}
}