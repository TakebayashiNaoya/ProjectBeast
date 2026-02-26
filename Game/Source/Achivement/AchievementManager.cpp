/**
 * @file AchievementManager.cpp
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementManager.h"
#include "Json/json.hpp"
#include "Source/Util/CRC32.h"
#include <fstream>


namespace
{
	/** アチーブメントのデータファイルのパス */
	const char* ACHIEVE_JSON_FILE_PATH = "Assets/parameter/achievement/achievementList.json";



	bool TryOpenJsonFile(const std::string& filePath, nlohmann::json& json)
	{
		nlohmann::json jsonTemp;
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			return false;
		}
		try
		{
			file >> jsonTemp;
		}
		catch (...)
		{
			return false;
		}
		json = std::move(jsonTemp);
		return true;
	}
}


namespace app
{
	namespace achievement
	{
		void AchievementManager::Start()
		{
			nlohmann::json json;
			// JSONの読み込みを試す
			if (!TryOpenJsonFile(m_achieveFilePath, json))
			{
				// 読み込み失敗
				return;
			}

			// JSON内の確認
			if (!json.contains("AchievementList")) return;
			if (!json["AchievementList"].contains("Achievement")) return;


			CreateAchievement(json["AchievementList"]["Achievement"]);
		}


		void AchievementManager::Update()
		{
		}


		void AchievementManager::Render(RenderContext& rc)
		{
		}


		AchievementManager::AchievementManager()
			: m_achieveFilePath(ACHIEVE_JSON_FILE_PATH)
		{
		}


		AchievementManager::~AchievementManager()
		{
			m_achievementMap.clear();
		}


		void AchievementManager::CreateAchievement(const nlohmann::json& json)
		{
			for (const auto& achieveData : json)
			{
				std::string type;
				achieveData.at("type").get_to(type);

				Achieve achieve;

				// タイプに応じてアチーブメントを作成


				if (type == "counter")
				{
					// カウンタータイプのアチーブメントを作成
					achieve = std::make_unique<CounterAchievement>();
				}
				if (type == "location")
				{
					// ロケーションタイプのアチーブメントを作成
					//achieve = std::make_unique<LocationAchievement>();
				}

				// アチーブメントを初期化
				achieve->Init(json);

				// 登録したキーを取得
				uint32_t key = achieve->GetID();
				// マップに追加
				m_achievementMap.emplace(key, std::move(achieve));
			}
		}


		AchievementManager* AchievementManager::m_instance = nullptr;
	}
}