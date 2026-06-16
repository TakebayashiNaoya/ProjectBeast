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

			// ① 渦潮の配置（ポジション）はJSONから読み込む
			nlohmann::json json; // 👈 変数名を「json」で定義しました
			if (util::JsonConverter::IsLoadJsonFile(json, positionsJsonPath))
			{
				LoadPositionMap(json);
			}

			// 座標JSONの最終更新時刻を記録する（デバッグビルドのみ）
#ifdef APP_DEBUG
			m_posLastWriteTime = util::JsonConverter::GetFileLastWriteTime(m_positionsJsonPath.c_str());
#endif // APP_DEBUG

			// ② パラメーターはバイナリファイルから読み込む
			core::ParameterManager::Get()->LoadParameterBinary<MasterWhirlpoolParameter>(
				parameterPath,
				[](std::istream& stream, MasterWhirlpoolParameter& p)
				{
					// 11個の float を順番に読み込む
					stream.read(reinterpret_cast<char*>(&p.whirlpoolRadius), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.attractSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.rotateSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.uvRotationSpeed), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.scaleChangeTime), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.stayTime), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.createInterval), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.orbitRadius), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.orbitRadiusVariation), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.orbitOffsetVariation), sizeof(float));
					stream.read(reinterpret_cast<char*>(&p.rotateScaleVariation), sizeof(float));
				}
			);

			// RenderingEngineに自身を登録する
			g_renderingEngine->RegisterNatureObject(this);

			// 座標JSONの最終更新時刻を記録する（デバッグビルドのみ）
#ifdef APP_DEBUG
			m_posLastWriteTime = util::JsonConverter::GetFileLastWriteTime(m_positionsJsonPath.c_str());
#endif // APP_DEBUG

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
				auto iter = m_whirlpoolMap.find(it);
				if (iter != m_whirlpoolMap.end())
				{
					m_whirlpoolMap.erase(iter);
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
			m_whirlpoolMap.clear();

			// パラメーターをアンロードする
			core::ParameterManager::Get()->UnloadParameter<MasterWhirlpoolParameter>();
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