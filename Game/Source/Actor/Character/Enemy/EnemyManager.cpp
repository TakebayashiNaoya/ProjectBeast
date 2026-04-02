/**
 * @file EnemyManager.cpp
 * @brief Enemyのマネージャー
 * @author 竹林
 */
#include "stdafx.h"
#include "EnemyManager.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "EnemyStateMachine.h"
#include "Source/Util/JsonConverter.h"
#include "Source/Actor/Stage/StageSystem.h"


namespace app
{
	namespace actor
	{
		EnemyManager* EnemyManager::m_instance = nullptr;


		EnemyManager::EnemyManager()
		{}


		EnemyManager::~EnemyManager()
		{
			// デストラクタで一括解放
			ClearEnemies();
		}


		void EnemyManager::ClearEnemies()
		{
			for (auto& data : m_enemyList)
			{
				delete data.controller;
				delete data.enemy;
			}
			m_enemyList.clear();
		}


		void EnemyManager::LoadEnemies(const nlohmann::json& json)
		{
			if (!json.contains("enemies")) return;

			/** JSONのエネミー配列をループ */
			for (const auto& enemyJson : json["enemies"])
			{
				EnemyData data;

				/** 1. エネミーの生成 */
				data.enemy = new Enemy();
				data.enemy->SetPosition(util::JsonConverter::ToVector3(enemyJson["spawnPosition"]));

				/** 2. 巣の座標を設定 */
				std::string homeName = util::JsonConverter::ToString(enemyJson["nestName"]);
				Vector3 homePos = actor::StageSystem::GetInstance()->GetObjectPosition(homeName);
				data.enemy->SetHomePosition(homePos);

				data.enemy->StartWrapper();

				/** 3. コントローラーの生成とターゲット設定 */
				data.controller = new EnemyController();
				data.controller->SetTarget(data.enemy);

				/** 4. 徘徊ルート(複数)の設定 */
				if (enemyJson.contains("patrolPoints"))
				{
					for (const auto& pointJson : enemyJson["patrolPoints"])
					{
						Vector3 point = util::JsonConverter::ToVector3(pointJson);
						data.controller->AddTargetPos(point);
					}
				}

				m_enemyList.push_back(data);
			}
		}


		void EnemyManager::Update()
		{
			for (auto& data : m_enemyList)
			{
				if (data.enemy) data.enemy->UpdateWrapper();
				if (data.controller) data.controller->Update();
			}
		}

		void EnemyManager::UpdateModelOnly()
		{
			for (auto& data : m_enemyList)
			{
				if (data.enemy)
				{
					// AIなどは動かさず、モデルのアニメーションと姿勢だけ更新する
					data.enemy->UpdateModelOnly();
				}
			}
		}


		void EnemyManager::Render(RenderContext& rc)
		{
			for (auto& data : m_enemyList)
			{
				if (data.enemy) data.enemy->RenderWrapper(rc);
			}
		}


		std::vector<Vector3> EnemyManager::GetPositionList() const
		{
			std::vector<Vector3> positionList;
			for (auto& data : m_enemyList)
			{
				positionList.push_back(data.enemy->GetTransform().m_position);
			}
			return positionList;
		}


		Enemy* EnemyManager::GetNearestSleepingEnemy(const Vector3& fromPosition, float maxRange) const
		{
			const float maxRangeSq = maxRange * maxRange;

			Enemy* nearest = nullptr;
			float minDistSq = maxRangeSq;

			for (const auto& data : m_enemyList)
			{
				if (!data.enemy) continue;

				auto* sm = data.enemy->GetEnemyStateMachine();

				/** クールダウン（睡眠）状態でなければ対象外 */
				if (!sm->IsCoolDown()) continue;

				const Vector3 diff = fromPosition - data.enemy->GetTransform().m_position;
				const float distSq = diff.LengthSq();

				/** 探索半径より遠ければスキップ */
				if (distSq > maxRangeSq) continue;

				if (distSq < minDistSq)
				{
					minDistSq = distSq;
					nearest = data.enemy;
				}
			}

			return nearest;
		}
	}
}