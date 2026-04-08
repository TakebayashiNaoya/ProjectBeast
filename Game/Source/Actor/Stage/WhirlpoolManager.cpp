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
			constexpr int MaxWhirlpoolNum = 3;
			constexpr int WhirlpoolIndex = 4;
			constexpr float WhirlpoolCreateInterval = 20.0f;
			constexpr float WhirlpoolY = 0.0f;




			/**
			 * @brief 渦潮の位置をランダムに選択する関数
			 * @return 渦潮の位置
			 */
			Vector3 SelectWhirlpoolPosition()
			{
				// 渦潮の位置をランダムに選択(番号なのでプラス1)
				const int index = (rand() % WhirlpoolIndex) + 1;
				// ステージオブジェクトの位置を取得
				Vector3 position = StageSystem::GetInstance()->GetObjectPosition("whirlpool" + std::to_string(index));
				position.y = WhirlpoolY;
				return position;
			}


			/**
			 * @brief 渦潮を生成する関数
			 */
			Whirlpool* CreateWhirlpool()
			{
				// 渦潮を生成する座標を取得
				Vector3 position = SelectWhirlpoolPosition();

				// 渦潮を生成
				auto* newWhirlpool = new Whirlpool();
				// 
				newWhirlpool->SetPosition(position);
				newWhirlpool->SetScale(Vector3(2.5f, 2.5f, 2.5f));
				newWhirlpool->StartWrapper();
				return newWhirlpool;
			}
		}


		void WhirlpoolManager::Start()
		{
			m_whirlpools.push_back(CreateWhirlpool());
		}


		void WhirlpoolManager::Update()
		{
			ForEach([&](Whirlpool* whirlpool)
				{
					whirlpool->UpdateWrapper();
				});
		}


		void WhirlpoolManager::Render(RenderContext& rc)
		{
			ForEach([&](Whirlpool* whirlpool)
				{
					whirlpool->RenderWrapper(rc);
				});
		}


		void WhirlpoolManager::ForEach(std::function<void(Whirlpool*)> cb)
		{
			if (m_whirlpools.empty()) return;

			for (auto* whirlpool : m_whirlpools)
			{
				cb(whirlpool);
			}
		}
	}
}