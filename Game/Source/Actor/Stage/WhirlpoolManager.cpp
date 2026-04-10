/**
 * @file WhirlpoolManager.cpp
 * @brief 渦潮を管理するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "StageSystem.h"
#include "Whirlpool.h"
#include "WhirlpoolManager.h"


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
			constexpr int WHIRLPOOL_INDEX = 4;
			/** 渦潮の生成間隔 */
			constexpr float WHIRLPOOL_CREATE_INTERVAL = 20.0f;
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

				int createNum = rand() % MAX_WHIRLPOOL_NUM;

				for (int i = 0; i < createNum; i++)
				{
					if (m_whirlpoolMap.size() >= MAX_WHIRLPOOL_NUM) break;

					CreateWhirlpool();
				}
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
					delete iter->second;
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


		void WhirlpoolManager::ForEach(std::function<void(Whirlpool* info)> cb)
		{
			if (m_whirlpoolMap.empty()) return;

			for (auto& it : m_whirlpoolMap)
			{
				if (!it.second) continue;

				cb(it.second);
			}
		}


		void WhirlpoolManager::CreateWhirlpool()
		{
			if (m_whirlpoolMap.size() >= MAX_WHIRLPOOL_NUM) return;

			bool isUsed = true;
			int index = -1;
			while (isUsed)
			{
				// 生成する座標が既に存在する渦潮の座標と被っていないか
				index = (rand() % WHIRLPOOL_INDEX) + 1;

				if (m_whirlpoolMap.empty()) break;

				for (auto& it : m_whirlpoolMap)
				{
					if (it.first == index) continue;

					isUsed = false;
				}
			}


			if (index == -1) return;


			// ステージオブジェクトの位置を取得
			Vector3 position = StageSystem::GetInstance()->GetObjectPosition("whirlpool" + std::to_string(index));
			position.y = WHIRLPOOL_Y;

			auto* newWhirlpool = new Whirlpool();
			// 渦潮を初期化
			newWhirlpool->SetIsNeedCollision(false);
			newWhirlpool->SetPosition(position);
			newWhirlpool->SetIndex(static_cast<uint8_t>(index));
			newWhirlpool->StartWrapper();
			m_whirlpoolMap.insert({ index, newWhirlpool });
		}
	}
}