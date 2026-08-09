/**
 * @file Actor.cpp
 * @brief ステージ上のオブジェクトを管理するシステム
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/JsonConverter.h"
#include "StageSystem.h"
#include <unordered_set>


 /**
  * @brief　ステージの情報を取得するための関数群
  */
namespace
{
	/** JSONのファイルネーム */
	const char* JSON_FILE_PATH = "Assets/parameter/stage/stageObject.json";
	/** ステージオブジェクトのキー */
	const char* STAGE_OBJECT_KEY = "StageObject";
	/** 地形設定のキー */
	const char* TERRAIN_CONFIG_KEY = "TerrainConfig";
	/** オブジェクト配列のキー */
	const char* OBJECT_ARRAY_KEY = "object";
	/** オブジェクトネームのキー */
	const char* OBJECT_NAME = "objectName";
	/** イグルーのキープレフィックス */
	const char* IGLOO_KEY_PREFIX = "igloo_";

	/** ステージ上に配置できる最大の数 : 512 */
	constexpr uint32_t MAX_OBJECT_NUM = 0x200;




	/**
	 * @brief ステージオブジェクトのトランスフォーム情報を読み込む
	 * @param object ステージオブジェクト
	 * @param json トランスフォーム情報のjson
	 */
	void LoadTransform(app::actor::IStageObject* object, const nlohmann::json& json)
	{
		using JC = app::util::JsonConverter;

		const bool isNeedCollision = JC::ToBool(json, "isNeedCollision");
		const Vector3 position = JC::ToVector3(json, "position");
		const Quaternion rotation = JC::ToRotation(json, "rotationDeg");
		const Vector3 scale = JC::ToVector3(json, "scale");

		// 影を落とすかどうか。指定が無ければ落とす
		object->SetCastShadow(json.value("castShadow", true));

		object->SetIsNeedCollision(isNeedCollision);
		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
		object->UpdateWrapper();
	}


	/**
	 * @brief ステージオブジェクトを初期化する
	 * @param object ステージオブジェクト
	 * @param item   jsonデータ
	 */
	void InitializeStageObject(app::actor::IStageObject* object, const nlohmann::json& item)
	{
		const std::string fileName = app::util::JsonConverter::ToString(item, "fileName");

		// pbrNameが指定されていればInit時に渡す（なければ空文字でデフォルト値を使用）
		std::string pbrName = "";
		if (item.contains("pbrName"))
		{
			pbrName = item["pbrName"].get<std::string>();
		}

		object->Init(fileName.c_str(), pbrName);

		LoadTransform(object, item);
		object->StartWrapper();
	}


	/**
	 * @brief JSON から TerrainConfig を解析する
	 */
	app::actor::TerrainObject::TerrainConfig ParseTerrainConfig(const nlohmann::json& j)
	{
		app::actor::TerrainObject::TerrainConfig cfg;
		if (!j.contains(TERRAIN_CONFIG_KEY)) return cfg;
		const auto& t = j[TERRAIN_CONFIG_KEY];
		cfg.totalWidth = t.value("totalWidth", cfg.totalWidth);
		cfg.totalDepth = t.value("totalDepth", cfg.totalDepth);
		cfg.heightScale = t.value("heightScale", cfg.heightScale);
		cfg.subsample = t.value("subsample", cfg.subsample);
		cfg.uvTile = t.value("uvTile", cfg.uvTile);
		cfg.albedoScale = t.value("albedoScale", cfg.albedoScale);
		cfg.yOffset = t.value("yOffset", cfg.yOffset);
		cfg.minHeight = t.value("minHeight", cfg.minHeight);
		cfg.chunkDivision = t.value("chunkDivision", cfg.chunkDivision);
		if (t.contains("heightmapPath"))
		{
			const std::string s = t["heightmapPath"].get<std::string>();
			cfg.heightmapPath.assign(s.begin(), s.end());
		}
		if (t.contains("splatmapPath"))
		{
			const std::string s = t["splatmapPath"].get<std::string>();
			cfg.splatmapPath.assign(s.begin(), s.end());
		}
		if (t.contains("pbrParam"))
		{
			const auto& p = t["pbrParam"];
			cfg.pbrParam.m_dirLightScale = p.value("dirLightScale", cfg.pbrParam.m_dirLightScale);
			cfg.pbrParam.m_ambientScale = p.value("ambientScale", cfg.pbrParam.m_ambientScale);
			cfg.pbrParam.m_metallicOffset = p.value("metallicOffset", cfg.pbrParam.m_metallicOffset);
			cfg.pbrParam.m_smoothOffset = p.value("smoothOffset", cfg.pbrParam.m_smoothOffset);
		}
		return cfg;
	}


	/**
	 * @brief jsonファイルの読み込みを試す
	 * @param json jsonファイル
	 * @param jsonFileName jsonのファイルパス
	 * @param time
	 */
	bool TryRoadJsonFile(nlohmann::json& json, const std::string& jsonFileName, time_t& time)
	{
		struct stat st;

		// ファイルの更新時間が変わっていなかった場合
		if (stat(jsonFileName.c_str(), &st) != 0
			|| time == st.st_mtime)
		{
			// 読み込む必要なし
			return false;
		}

		// jsonファイルを読み込む
		if (!app::util::JsonConverter::IsLoadJsonFile(json, jsonFileName))
		{
			// 読み込み失敗
			return false;
		}

		// 更新時間を保存
		time = st.st_mtime;

		// 読み込み成功
		return true;
	}
}


namespace app
{
	namespace actor
	{
		void StageSystem::CreateStageObject(const nlohmann::json& json)
		{
			// JSON内の確認
			for (const auto& objData : json[STAGE_OBJECT_KEY][OBJECT_ARRAY_KEY])
			{
				// オブジェクトキーを取得
				const ObjectKey objectKey = app::util::JsonConverter::ToString(objData, OBJECT_NAME);

				if (!m_objectMap.empty())
				{
					// マップ内を検索
					auto it = m_objectMap.find(objectKey);
					// 既存のものがあればスキップ
					if (it != m_objectMap.end()) continue;
				}

				// ここに来たということは既存のものはないので生成
				Object newObject = std::make_unique<IStageObject>();
				// オブジェクトを初期化
				InitializeStageObject(newObject.get(), objData);
				// マップに追加
				m_objectMap.emplace(objectKey, std::move(newObject));
			}
		}


		void StageSystem::DeleteStageObject(const nlohmann::json& json)
		{
			// オブジェクトマップが保持しているをキーを保存するための配列
			std::unordered_set<ObjectKey> toBeDeleted;

			// オブジェクトマップが空の場合は処理しない
			if (m_objectMap.empty()) return;


			// 保持しているキーをすべて保存
			for (const auto& obj : m_objectMap)
			{
				toBeDeleted.insert(obj.first);
			}

			// JSON内のオブジェクトキーをもとに削除対象から外す
			for (const auto& objData : json[STAGE_OBJECT_KEY][OBJECT_ARRAY_KEY])
			{
				// オブジェクトキーを取得
				const ObjectKey objectKey = app::util::JsonConverter::ToString(objData, OBJECT_NAME);
				toBeDeleted.erase(objectKey);
			}

			// 削除対象のキーをもとにオブジェクトを削除
			for (const ObjectKey& key : toBeDeleted)
			{
				m_objectMap.erase(key);
			}
		}


		void StageSystem::ReloadTransform(const nlohmann::json& j)
		{
			// 必要なデータが存在するかチェック
			if (!j.contains(STAGE_OBJECT_KEY)) return;
			if (!j[STAGE_OBJECT_KEY].contains(OBJECT_ARRAY_KEY)) return;

			for (const auto& objData : j[STAGE_OBJECT_KEY][OBJECT_ARRAY_KEY])
			{
				// オブジェクトキーを取得
				const ObjectKey& objKey = app::util::JsonConverter::ToString(objData, OBJECT_NAME);

				auto it = m_objectMap.find(objKey);
				// 既存のものがなければスキップ
				if (it == m_objectMap.end()) continue;

				LoadTransform(it->second.get(), objData);
			}
		}


		void StageSystem::LoadEnemyNests(const nlohmann::json& json)
		{
			if (!json.contains("enemies")) return;

			for (const auto& enemyJson : json["enemies"]) {
				Vector3 nestPos = util::JsonConverter::ToVector3(enemyJson["nestPosition"]);
			}
		}


		bool StageSystem::IsAllLoaded() const
		{
			for (const auto& obj : m_objectMap)
			{
				if (!obj.second->IsLoaded()) return false;
			}
			return !m_objectMap.empty();
		}


		uint8_t StageSystem::GetNumbaringObjectCount(const std::string& objectKey) const
		{
			uint8_t count = 0;
			const std::string prefix = objectKey + "_";

			for (const auto& obj : m_objectMap)
			{
				if (obj.first.find(prefix) == 0)
				{
					count++;
				}
			}
			return count;
		}

		StageSystem::StageSystem()
		{
			/** マップのメモリを確保 */
			m_objectMap.reserve(MAX_OBJECT_NUM);
		}


		void StageSystem::LoadStageObjectsFromJson(const std::string& jsonPath)
		{
			nlohmann::json json;
			if (app::util::JsonConverter::IsLoadJsonFile(json, jsonPath))
			{
				CreateStageObject(json);
			}

#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			m_stageObjectJsonPath = jsonPath;
			struct stat st;
			if (stat(jsonPath.c_str(), &st) == 0)
			{
				m_lastUpdateTime = st.st_mtime;
			}
#endif
		}


		void StageSystem::InitTerrain(const TerrainObject::TerrainConfig& config)
		{
			m_terrain = std::make_unique<TerrainObject>();
			m_terrain->Init(config);
		}


		void StageSystem::InitTerrainFromJson(const std::string& jsonPath)
		{
			nlohmann::json json;
			if (!app::util::JsonConverter::IsLoadJsonFile(json, jsonPath))
			{
				return;
			}

			TerrainObject::TerrainConfig config = ParseTerrainConfig(json);

#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			m_terrainJsonPath = jsonPath;
			struct stat st;
			if (stat(jsonPath.c_str(), &st) == 0)
			{
				m_lastTerrainUpdateTime = st.st_mtime;
			}
#endif

			InitTerrain(config);
		}


		void StageSystem::Update()
		{
			if (m_terrain) { m_terrain->UpdateWrapper(); }

			for (const auto& obj : m_objectMap)
			{
				obj.second->UpdateWrapper();
			}

#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD

			// ステージオブジェクトのホットリロード
			if (!m_stageObjectJsonPath.empty())
			{
				nlohmann::json json;
				if (TryRoadJsonFile(json, m_stageObjectJsonPath, m_lastUpdateTime))
				{
					if (json.contains(STAGE_OBJECT_KEY) && json[STAGE_OBJECT_KEY].contains(OBJECT_ARRAY_KEY))
					{
						DeleteStageObject(json);
						CreateStageObject(json);
						ReloadTransform(json);
					}
				}
			}

			// 地形パラメータのホットリロード
			if (!m_terrainJsonPath.empty())
			{
				nlohmann::json terrainJson;
				if (TryRoadJsonFile(terrainJson, m_terrainJsonPath, m_lastTerrainUpdateTime))
				{
					InitTerrain(ParseTerrainConfig(terrainJson));
				}
			}

#endif // ENABLE_OBJECT_LAYOUT_HOTRELOAD
		}


		void StageSystem::Render(RenderContext& rc)
		{
			if (m_terrain) { m_terrain->RenderWrapper(rc); }

			for (const auto& obj : m_objectMap)
			{
				obj.second->RenderWrapper(rc);
			}
		}


		Vector3 StageSystem::GetObjectPosition(const std::string& key) const
		{
			auto it = m_objectMap.find(key);
			if (it != m_objectMap.end())
			{
				return it->second->GetTransform().m_position;
			}

			// 見つからなかった場合
			return Vector3::Zero;
		}


		Quaternion StageSystem::GetObjectRotation(const std::string& key) const
		{
			auto it = m_objectMap.find(key);
			if (it != m_objectMap.end())
			{
				return it->second->GetTransform().m_rotation;
			}

			// 見つからなかった場合は無回転を返す
			return Quaternion::Identity;
		}


		StageSystem::ObjectMap::const_iterator StageSystem::FindNearestByPrefix(
			const char* prefix, const Vector3& from) const
		{
			auto nearest = m_objectMap.end();
			float minDistSq = FLT_MAX;

			for (auto it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
			{
				if (it->first.find(prefix) != 0) continue;

				const Vector3 diff = it->second->GetTransform().m_position - from;
				const float distSq = diff.LengthSq();

				if (distSq < minDistSq)
				{
					minDistSq = distSq;
					nearest = it;
				}
			}

			return nearest;
		}


		Vector3 StageSystem::GetNearestIglooPosition(const Vector3& from) const
		{
			auto it = FindNearestByPrefix(IGLOO_KEY_PREFIX, from);
			return (it != m_objectMap.end())
				? it->second->GetTransform().m_position
				: Vector3::Zero;
		}


		Quaternion StageSystem::GetNearestIglooRotation(const Vector3& from) const
		{
			auto it = FindNearestByPrefix(IGLOO_KEY_PREFIX, from);
			return (it != m_objectMap.end())
				? it->second->GetTransform().m_rotation
				: Quaternion::Identity;
		}


		void StageSystem::BreakIgloo(const std::string& key)
		{
			auto it = m_objectMap.find(key);
			if (it != m_objectMap.end())
			{
				// 1. 当たり判定をオフにする（物理的な破壊）
				it->second->SetIsNeedCollision(false);
				// 姿を消す（非表示にする）
				it->second->SetIsVisible(false);

				// 2. かまくらの中心座標を取得（演出用）
				Vector3 iglooPos = it->second->GetTransform().m_position;

				// 3. かまくら破壊時のサウンド再生
				SoundManager::Get().PlaySE(enSoundKind_IglooBreak);

				// 4. かまくら破壊時のエフェクト再生
				// ※ 登録名が EnEffectKind::IglooBreak の場合。環境に合わせて調整してください。

				const Vector3 effectScale(30.0f, 30.0f, 30.0f); // かまくらの大きさに合わせて調整

				EffectManager::Get().PlayEffect(
					EnEffectKind::IglooBreak,
					iglooPos,
					Quaternion::Identity,
					effectScale // かまくらの大きさに合わせて調整
				);



				// 指定したキー（例: "igloo_01"）だけをピンポイントで消去します。
				m_objectMap.erase(it);
			}
		}


		std::string StageSystem::GetNearestIglooKey(const Vector3& from) const
		{
			auto it = FindNearestByPrefix(IGLOO_KEY_PREFIX, from);
			return (it != m_objectMap.end()) ? it->first : std::string{};
		}


		Vector3 StageSystem::GetNearestBearNestPosition(const Vector3& from) const
		{
			auto it = FindNearestByPrefix("bearHome_", from);
			return (it != m_objectMap.end())
				? it->second->GetTransform().m_position
				: Vector3::Zero;
		}


		/** インスタンスを初期化 */
		StageSystem* StageSystem::m_instance = nullptr;
	}
}