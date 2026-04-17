/**
 * @file WhirlpoolManager.cpp
 * @brief 渦潮を管理するクラス
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "Whirlpool.h"
#include "WhirlpoolManager.h"
#include "Source/Util/JsonConverter.h"
#include <algorithm>
#include <random>


namespace app
{
	namespace nature
	{
		WhirlpoolManager* WhirlpoolManager::m_instance = nullptr;

		namespace
		{
			/** 渦潮の座標JSONのパス */
			const char* WHIRLPOOL_POSITIONS_JSON_PATH = "Assets/parameter/stage/whirlpoolPositions.json";
			/** 渦潮の位置のキー */
			const char* WHIRLPOOL_POSITIONS_KEY = "whirlpoolPositions";
			/** 渦潮のインデックスのキー */
			const char* WHIRLPOOL_INDEX_KEY = "index";
			/** 渦潮の座標のキー */
			const char* WHIRLPOOL_POSITION_KEY = "position";

			/** 渦潮の生成間隔 */
			constexpr float WHIRLPOOL_CREATE_INTERVAL = 2.0f;
			/** 渦潮のY座標 */
			constexpr float WHIRLPOOL_Y = 0.0f;
		}


		void WhirlpoolManager::Start()
		{
			// 渦潮の座標JSONを読み込む
			nlohmann::json json;
			if (!util::JsonConverter::IsLoadJsonFile(json, WHIRLPOOL_POSITIONS_JSON_PATH))
			{
				K2_ASSERT(false, "whirlpoolPositions.jsonの読み込みに失敗しました");
				return;
			}

			// 座標をインデックスをキーとしてマップに格納する
			for (const auto& entry : json[WHIRLPOOL_POSITIONS_KEY])
			{
				const uint8_t index = static_cast<uint8_t>(entry[WHIRLPOOL_INDEX_KEY].get<int>());
				Vector3       position = util::JsonConverter::ToVector3(entry[WHIRLPOOL_POSITION_KEY]);
				position.y = WHIRLPOOL_Y;
				m_positionMap.emplace(index, position);
			}
		}


		void WhirlpoolManager::Update()
		{
			// 渦潮の生成タイマーを更新する
			m_timer += g_gameTime->GetFrameDeltaTime();
			if (m_timer >= WHIRLPOOL_CREATE_INTERVAL)
			{
				m_timer = 0.0f;
				CreateWhirlpool();
			}

			std::vector<uint8_t> removeIndexes;

			ForEach([&](Whirlpool* info)
				{
					// 渦潮の状態がNoneなら削除する
					if (info->GetState() == Whirlpool::EnWhirlpoolState::None)
					{
						removeIndexes.push_back(info->GetIndex());
					}

					info->UpdateWrapper();
				});

			for (auto& it : removeIndexes)
			{
				auto iter = m_whirlpoolMap.find(it);
				if (iter != m_whirlpoolMap.end())
				{
					m_whirlpoolMap.erase(iter);
				}
			}
		}


		void WhirlpoolManager::Render(RenderContext& rc)
		{
			ForEach([&](Whirlpool* whirlpool)
				{
					whirlpool->RenderWrapper(rc);
				});
		}


		WhirlpoolManager::WhirlpoolManager()
			: m_timer(0.0f)
		{}


		WhirlpoolManager::~WhirlpoolManager()
		{
			m_whirlpoolMap.clear();
		}


		void WhirlpoolManager::ForEach(std::function<void(Whirlpool* info)> cb)
		{
			if (m_whirlpoolMap.empty()) return;

			for (auto& it : m_whirlpoolMap)
			{
				if (!it.second) continue;
				cb(it.second.get());
			}
		}


		void WhirlpoolManager::CreateWhirlpool()
		{
			// 生成済みでないインデックスを候補として収集する
			std::vector<uint8_t> candidates;
			for (const auto& entry : m_positionMap)
			{
				if (m_whirlpoolMap.find(entry.first) == m_whirlpoolMap.end())
				{
					candidates.push_back(entry.first);
				}
			}

			if (candidates.empty()) return;

			// ランダムにインデックスを選択する
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size() - 1));

			const uint8_t  index = candidates[dist(gen)];
			const Vector3& position = m_positionMap.at(index);

			auto newWhirlpool = std::make_unique<Whirlpool>();
			newWhirlpool->SetPosition(position);
			newWhirlpool->SetIndex(index);
			newWhirlpool->StartWrapper();
			m_whirlpoolMap.insert({ index, std::move(newWhirlpool) });
		}
	}
}