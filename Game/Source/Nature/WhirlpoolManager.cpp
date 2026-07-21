/**
 * @file WhirlpoolManager.cpp
 * @brief 渦潮を管理するクラス
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"
#include "Whirlpool.h"
#include "WhirlpoolManager.h"
#include "WhirlpoolParameter.h"
#include <algorithm>
#include <random>
#include <sys/stat.h>


namespace app
{
	namespace nature
	{
		WhirlpoolManager* WhirlpoolManager::m_instance = nullptr;

		namespace
		{

			/** 渦潮の位置のキー */
			const char* WHIRLPOOL_POSITIONS_KEY = "whirlpoolPositions";
			/** 渦潮のインデックスのキー */
			const char* WHIRLPOOL_INDEX_KEY = "index";
			/** 渦潮の座標のキー */
			const char* WHIRLPOOL_POSITION_KEY = "position";

			/** 渦潮のY座標 */
			constexpr float WHIRLPOOL_Y = 0.0f;


			/**
			 * @brief JSONファイルが更新されていれば読み込む
			 * @param json      読み込んだJSONを格納する変数
			 * @param filePath  ファイルパス
			 * @param lastTime  前回の更新時刻（更新があれば書き換える）
			 * @return 読み込んだ場合true
			 */
			bool TryReloadJsonFile(nlohmann::json& json, const char* filePath, time_t& lastTime)
			{
				struct stat st;

				// ファイル情報の取得に失敗、または更新時刻が変わっていない場合はスキップ
				if (stat(filePath, &st) != 0 || lastTime == st.st_mtime)
				{
					return false;
				}

				// JSONの読み込みを試みる
				if (!util::JsonConverter::IsLoadJsonFile(json, filePath))
				{
					return false;
				}

				// 更新時刻を記録する
				lastTime = st.st_mtime;
				return true;
			}
		}


		void WhirlpoolManager::Start(const char* positionsJsonPath, const char* parameterPath)
		{
			m_positionsJsonPath = positionsJsonPath;

			nlohmann::json json;
			if (util::JsonConverter::IsLoadJsonFile(json, positionsJsonPath))
			{
				LoadPositionMap(json);
			}

			// 座標JSONの最終更新時刻を記録する（デバッグビルドのみ）
#ifdef APP_DEBUG
			m_posLastWriteTime = util::JsonConverter::GetFileLastWriteTime(m_positionsJsonPath.c_str());
#endif // APP_DEBUG

			core::ParameterManager::Get()->LoadParameterBinary<MasterWhirlpoolParameter>(parameterPath);

			// RenderingEngineに自身を登録する
			g_renderingEngine->RegisterNatureObject(this);
		}


		void WhirlpoolManager::Update()
		{
			// 座標JSONのホットリロードを試みる（デバッグビルドのみ）
#ifdef APP_DEBUG
			nlohmann::json posJson;
			if (TryReloadJsonFile(posJson, m_positionsJsonPath.c_str(), m_posLastWriteTime))
			{
				LoadPositionMap(posJson);
			}
#endif // APP_DEBUG

			// 渦潮の生成タイマーを更新する
			const MasterWhirlpoolParameter* param = core::ParameterManager::Get()->GetParameter<MasterWhirlpoolParameter>();
			const float                     createInterval = (param != nullptr) ?
				param->createInterval : 2.0f;

			m_timer += g_gameTime->GetFrameDeltaTime();
			if (m_timer >= createInterval)
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
				if (it < m_whirlpoolSlots.size())
				{
					m_whirlpoolSlots[it].reset();
				}
			}
		}


		void WhirlpoolManager::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
		{
			ForEach([&](Whirlpool* whirlpool)
				{
					// 球判定で視錐台外の渦潮をスキップする
					// 中心座標はトランスフォームの位置、半径はメッシュ半径×最大スケールXZで近似する
					const Vector3& center = whirlpool->GetTransform().m_position;
					const float    radius = whirlpool->GetMaxScaleXZ() * Whirlpool::MESH_RADIUS;

					if (!view.frustum.IsIntersectSphere(center, radius))
					{
						return;
					}

					// 球判定を通過した渦潮に対してトライアングルカリングを適用して描画する
					whirlpool->Render(rc, view);
				});
		}


		WhirlpoolManager::WhirlpoolManager()
			: m_timer(0.0f)
			, m_posLastWriteTime(0)
		{}


		WhirlpoolManager::~WhirlpoolManager()
		{
			// RenderingEngineから自身の登録を解除する
			g_renderingEngine->UnregisterNatureObject(this);
			m_whirlpoolSlots.clear();

			// パラメーターをアンロードする
			core::ParameterManager::Get()->UnloadParameter<MasterWhirlpoolParameter>();
		}


		void WhirlpoolManager::ForEach(std::function<void(Whirlpool* info)> cb)
		{
			for (auto& whirlpool : m_whirlpoolSlots)
			{
				if (!whirlpool) continue;
				cb(whirlpool.get());
			}
		}


		void WhirlpoolManager::NotifyPenguinDestroyed(actor::ChildPenguin* penguin)
		{
			ForEach([&](Whirlpool* whirlpool)
				{
					whirlpool->NotifyPenguinDestroyed(penguin);
				});
		}


		void WhirlpoolManager::LoadPositionMap(const nlohmann::json& json)
		{
			if (!json.contains(WHIRLPOOL_POSITIONS_KEY)) return;

			m_positionMap.clear();

			// 座標をインデックスをキーとしてマップに格納する
			for (const auto& entry : json[WHIRLPOOL_POSITIONS_KEY])
			{
				const uint8_t index = static_cast<uint8_t>(entry[WHIRLPOOL_INDEX_KEY].get<int>());
				Vector3       position = util::JsonConverter::ToVector3(entry[WHIRLPOOL_POSITION_KEY]);
				position.y = WHIRLPOOL_Y;
				m_positionMap.emplace(index, position);
			}
		}


		void WhirlpoolManager::CreateWhirlpool()
		{
			// 生成済みでないインデックスを候補として収集する
			std::vector<uint8_t> candidates;
			for (const auto& entry : m_positionMap)
			{
				const bool isOccupied = entry.first < m_whirlpoolSlots.size() && m_whirlpoolSlots[entry.first];
				if (!isOccupied)
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

			// 配置インデックスをそのまま添字として使う（既存の渦潮の添字がずれないようにするため）
			if (m_whirlpoolSlots.size() <= index)
			{
				m_whirlpoolSlots.resize(index + 1);
			}
			m_whirlpoolSlots[index] = std::move(newWhirlpool);
		}
	}
}