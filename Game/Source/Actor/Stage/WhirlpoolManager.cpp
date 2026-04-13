/**
 * @file WhirlpoolManager.cpp
 * @brief 渦潮を管理するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSystem.h"
#include "Whirlpool.h"
#include "WhirlpoolManager.h"
#include <algorithm>
#include <random>


namespace app
{
	namespace actor
	{

		WhirlpoolManager* WhirlpoolManager::m_instance = nullptr;

		namespace
		{
			/** 最大渦潮数 */
			constexpr int MAX_WHIRLPOOL_NUM = 3;
			/** 渦潮の位置の数 */
			constexpr int MIN_WHIRLPOOL_INDEX = 1;
			constexpr int MAX_WHIRLPOOL_INDEX = 4;
			/** 渦潮の生成間隔 */
			constexpr float WHIRLPOOL_CREATE_INTERVAL = 5.0f;
			/** 渦潮のY座標 */
			constexpr float WHIRLPOOL_Y = 0.0f;
		}


		void WhirlpoolManager::Start()
		{}


		void WhirlpoolManager::Update()
		{
			// 渦潮の生成タイマーを更新
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
			// 既に最大数の渦潮が存在する場合は生成しない
			if (m_whirlpoolMap.size() >= MAX_WHIRLPOOL_NUM) return;

			std::vector<uint8_t> candidates;
			for (int i = MIN_WHIRLPOOL_INDEX; i <= MAX_WHIRLPOOL_INDEX; ++i)
			{
				if (m_whirlpoolMap.find(i) == m_whirlpoolMap.end())
				{
					candidates.push_back(i);
				}
			}

			if (candidates.empty()) return;


			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size() - 1));

			uint8_t index = candidates[dist(gen)];


			// ステージオブジェクトの位置を取得
			Vector3 position = StageSystem::GetInstance()->GetObjectPosition("whirlpool" + std::to_string(index));
			position.y = WHIRLPOOL_Y;

			auto newWhirlpool = std::make_unique<Whirlpool>();
			// 渦潮を初期化
			newWhirlpool->SetIsNeedCollision(false);
			newWhirlpool->SetPosition(position);
			newWhirlpool->SetIndex(static_cast<uint8_t>(index));
			newWhirlpool->StartWrapper();
			m_whirlpoolMap.insert({ index, std::move(newWhirlpool) });
		}
	}
}