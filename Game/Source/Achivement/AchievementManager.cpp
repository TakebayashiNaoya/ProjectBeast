/**
 * @file AchievementManager.cpp
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementManager.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"


namespace
{
	/** アチーブメントのデータファイルのパス */
	const char* ACHIEVE_JSON_FILE_PATH = "Assets/parameter/achievement/achievementList.json";

}


namespace app
{
	namespace achievement
	{
		void AchievementManager::Start()
		{
			nlohmann::json json;
			// JSONの読み込みを試す
			if (!app::util::JsonConverter::IsLoadJsonFile(json, JSON_FILE_PATH))
			{
				// 読み込み失敗
				return;
			}

			// JSON内の確認
			if (!json.contains(ACHIEVE_LIST_KEY)) return;
			if (!json[ACHIEVE_LIST_KEY].contains(ACHIEVE_KEY)) return;


			CreateAchievement(json[ACHIEVE_LIST_KEY][ACHIEVE_KEY]);
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
				// タイプのキーが存在しない場合はエラー
				K2_ASSERT(achieveData.contains("type"), "typeが未設定");
				// タイプを取得
				const std::string type = app::util::JsonConverter::ToString(achieveData["type"]);

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