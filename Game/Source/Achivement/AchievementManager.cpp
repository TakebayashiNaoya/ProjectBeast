/**
 * @file AchievementManager.cpp
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "AchievementManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Util/CRC32.h"
#include "Source/Util/JsonConverter.h"


namespace
{
	/** アチーブメントのデータファイルのパス */
	const char* JSON_FILE_PATH = "Assets/parameter/achievement/AchievementList.json";

	/** アチーブメントリストのキー */
	const char* ACHIEVE_LIST_KEY = "AchievementList";
	/** アチーブメントのキー */
	const char* ACHIEVE_KEY = "Achievement";
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

				// ★ json ではなく achieveData から取得するように修正
				std::string type = app::util::JsonConverter::ToString(achieveData["type"]);

				// conditionやtargetValueが設定されていない場合のエラー回避
				std::string conditionStr = "";
				if (achieveData.contains("condition")) {
					conditionStr = app::util::JsonConverter::ToString(achieveData["condition"]);
				}

				uint32_t targetValue = 0;
				if (achieveData.contains("targetValue")) {
					targetValue = app::util::JsonConverter::ToUInt32(achieveData["targetValue"]);
				}

				Achieve newAchieve; // std::unique_ptr<AchievementBase> と同義

				// タイプに応じてアチーブメントを作成
				if (type == "Condition")
				{
					auto conditionAchieve = std::make_unique<ConditionAchievement>();

					if (conditionStr == "CheckRescuedCount")
					{
						// 子ペンギンを90匹以上集めたか判定
						conditionAchieve->SetCondition([targetValue]() {
							return app::actor::ChildPenguinManager::GetInstance()->GetRescuedNum() >= static_cast<int>(targetValue);
							});
					}
					else if (conditionStr == "CheckSimultaneousChase")
					{
						// 2頭以上のシロクマに【同時に】隊列ペンギンを追われているか判定
						// GetEnemies() と GetControllers() は同じ順序で返るため、
						// インデックスを合わせてエネミーとコントローラーを対応させる
						conditionAchieve->SetCondition([targetValue]() {
							auto* em = app::actor::EnemyManager::GetInstance();
							auto* cm = app::actor::ChildPenguinManager::GetInstance();
							if (!em || !cm) return false;

							auto enemies = em->GetEnemies();
							auto controllers = em->GetControllers();
							const size_t count = enemies.size();

							int chaseCount = 0;
							for (size_t idx = 0; idx < count; ++idx)
							{
								auto* enemy = enemies[idx];
								auto* controller = (idx < controllers.size()) ? controllers[idx] : nullptr;
								if (!enemy || !controller) continue;

								auto* sm = enemy->GetEnemyStateMachine();
								if (!sm) continue;

								// Chase中かつ追跡対象が隊列ペンギンであるものだけカウント
								if (sm->IsChasing())
								{
									const auto* found = controller->GetFoundPenguin();
									if (found != nullptr && cm->IsFollower(found))
									{
										chaseCount++;
									}
								}
							}
							return chaseCount >= static_cast<int>(targetValue);
							});
					}
					newAchieve = std::move(conditionAchieve);
				}
				else if (type == "Counter" || type == "counter")
				{
					auto counterAchieve = std::make_unique<CounterAchievement>();

					if (conditionStr == "CheckIndividualChase")
					{
						// 3頭それぞれが一度でも隊列ペンギンを追跡したかをカウント
						counterAchieve->SetCondition([targetValue]() {
							auto* em = app::actor::EnemyManager::GetInstance();
							if (!em) return false;

							uint32_t chasedCount = 0;
							for (auto* controller : em->GetControllers())
							{
								if (controller && controller->HasChased())
								{
									chasedCount++;
								}
							}
							return chasedCount >= targetValue;
							});
					}
					newAchieve = std::move(counterAchieve);
				}
				else if (type == "Event")
				{
					newAchieve = std::make_unique<EventAchievement>();
				}
				else if (type == "Record")
				{
					newAchieve = std::make_unique<RecordAchievement>();
				}
				else if (type == "Location" || type == "location")
				{
					newAchieve = std::make_unique<LocationAchievement>();
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


		AchievementManager* AchievementManager::m_instance = nullptr;
	}
}