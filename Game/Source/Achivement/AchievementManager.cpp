/**
 * @file AchievementManager.cpp
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/UI/Layout.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"


namespace
{
	/** アチーブメントリストのキー */
	const char* ACHIEVE_LIST_KEY = "AchievementList";
	/** アチーブメントのキー */
	const char* ACHIEVE_KEY = "Achievement";
}


namespace app
{
	namespace achievement
	{
		void AchievementManager::Start(const char* jsonPath)
		{
			m_jsonPath = jsonPath;
#ifdef APP_DEBUG
			m_lastUpdateTime = app::util::JsonConverter::GetFileLastWriteTime(jsonPath);
#endif

			nlohmann::json json;
			// JSONの読み込みを試す
			if (!app::util::JsonConverter::IsLoadJsonFile(json, jsonPath))
			{
				// 読み込み失敗
				return;
			}

			// JSON内の確認
			if (!json.contains(ACHIEVE_LIST_KEY)) return;
			if (!json[ACHIEVE_LIST_KEY].contains(ACHIEVE_KEY)) return;


			CreateAchievement(json[ACHIEVE_LIST_KEY][ACHIEVE_KEY]);
		}


		void AchievementManager::CheckHotReload()
		{
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			if (!m_jsonPath.empty() && app::util::JsonConverter::CheckFileModified(m_jsonPath, m_lastUpdateTime))
			{
				m_lastUpdateTime = app::util::JsonConverter::GetFileLastWriteTime(m_jsonPath.c_str());

				m_achievementList.clear();
				m_achievementMap.clear();
				// デバッグ専用のホットリロードのため、ゲーム中のカウンタもリセットして
				// 再読み込みしたアチーブメント定義を最初から評価し直す
				m_bearKillCount         = 0;
				m_whirlpoolCaptureCount = 0;

				nlohmann::json json;
				if (app::util::JsonConverter::IsLoadJsonFile(json, m_jsonPath))
				{
					if (json.contains(ACHIEVE_LIST_KEY) && json[ACHIEVE_LIST_KEY].contains(ACHIEVE_KEY))
					{
						CreateAchievement(json[ACHIEVE_LIST_KEY][ACHIEVE_KEY]);
					}
				}
				m_reloadVersion++;
			}
#endif
		}


		void AchievementManager::Update()
		{
			CheckHotReload();

			for (auto& pair : m_achievementMap)
			{
				if (pair.second)
				{
					pair.second->Update();
				}
			}
		}


		void AchievementManager::Render(RenderContext& rc)
		{}


		AchievementManager::AchievementManager()
		{}


		AchievementManager::~AchievementManager()
		{
			m_achievementList.clear(); // こちらが所有権を持つ
			m_achievementMap.clear();  // 生ポインタなのでclearだけでOK
		}


		void AchievementManager::CreateAchievement(const nlohmann::json& json)
		{
			for (const auto& achieveData : json)
			{
				// タイプのキーが存在しない場合はエラー
				K2_ASSERT(achieveData.contains("type"), "typeが未設定");

				std::string type = app::util::JsonConverter::ToString(achieveData, "type");

				std::string conditionStr;
				if (achieveData.contains("condition"))
					conditionStr = app::util::JsonConverter::ToString(achieveData, "condition");

				uint32_t targetValue = 0;
				if (achieveData.contains("targetValue"))
					targetValue = app::util::JsonConverter::ToUInt32(achieveData, "targetValue");

				Achieve newAchieve; // std::unique_ptr<AchievementBase> と同義

				// タイプに応じてアチーブメントを作成
				if (type == "Condition")
				{
					auto conditionAchieve = std::make_unique<ConditionAchievement>();

					if (conditionStr == "CheckRescuedCount")
					{
						// 子ペンギンを規定数以上集めたか判定
						conditionAchieve->SetCondition([targetValue]() {
							return app::actor::ChildPenguinManager::GetInstance()->GetRescuedNum() >= static_cast<int>(targetValue);
							});
					}
					newAchieve = std::move(conditionAchieve);
				}
				else if (type == "Event")
				{
					newAchieve = std::make_unique<EventAchievement>();
				}
				else if (type == "FinalCondition")
				{
					auto finalAchieve = std::make_unique<FinalConditionAchievement>();

					if (conditionStr == "CheckBearKillsAtMost")
					{
						finalAchieve->SetCondition([this, targetValue]() {
							return m_bearKillCount <= static_cast<int>(targetValue);
						});
					}
					else if (conditionStr == "CheckWhirlpoolCapturesAtMost")
					{
						finalAchieve->SetCondition([this, targetValue]() {
							return m_whirlpoolCaptureCount <= static_cast<int>(targetValue);
						});
					}
					newAchieve = std::move(finalAchieve);
				}

				// アチーブメントを初期化して登録
				if (newAchieve)
				{
					newAchieve->Init(achieveData);
					newAchieve->SetIndex(static_cast<int>(m_achievementList.size())); // ← インデックスをセット

					uint32_t key = newAchieve->GetID();
					m_achievementMap.emplace(key, newAchieve.get()); // 生ポインタをmapに
					m_achievementList.push_back(std::move(newAchieve)); // 所有権はvectorが持つ
				}
			}
		}


		std::vector<AchievementBase*> AchievementManager::GetAllAchievements() const
		{
			std::vector<AchievementBase*> allList;
			for (const auto& achieve : m_achievementList) // vectorなのでJSON順が保証される
			{
				if (achieve)
				{
					allList.push_back(achieve.get());
				}
			}
			return allList;
		}


		AchievementBase* AchievementManager::GetAchievement(uint32_t id)
		{
			auto it = m_achievementMap.find(id);
			if (it != m_achievementMap.end())
			{
				return it->second;
			}
			return nullptr;
		}


		void AchievementManager::FinalizeAchievements()
		{
			for (auto& pair : m_achievementMap)
			{
				if (pair.second) pair.second->Finalize();
			}
		}


		AchievementManager* AchievementManager::m_instance = nullptr;
	}
}