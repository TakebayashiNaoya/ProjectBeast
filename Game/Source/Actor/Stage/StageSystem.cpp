/**
 * @file Actor.cpp
 * @brief ステージ上のオブジェクトを管理するシステム
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"
#include "StageSystem.h"
#include <unordered_set>




 /**
  * @brief　ステージの情報を取得するための関数群
  */
namespace
{
	/** JSONのファイルネーム */
	const char* JSON_FILE_NAME = "Assets/parameter/stage/stageObject.json";
	/** ステージオブジェクトのキー */
	const char* STAGE_OBJECT_KEY = "StageObject";
	/** オブジェクト配列のキー */
	const char* OBJECT_ARRAY_KEY = "object";
	/** オブジェクトネームのキー */
	const char* OBJECT_NAME = "objectName";

	/** ステージ上に配置できる最大の数 : 512 */
	constexpr uint32_t MAX_OBJECT_NUM = 0x200;


	/**
	 * @brief jsonファイルからVector3を変換する
	 * @param arr json配列
	 */
	Vector3 ParseVector3(const nlohmann::json& arr)
	{
		// 配列のサイズと型をチェック
		if (arr.size() != 3 || !arr.is_number())
		{
			return Vector3::Zero;
		}

		return Vector3(
			arr[0].get<float>(),
			arr[1].get<float>(),
			arr[2].get<float>()
		);
	}


	/**
	 * @brief jsonファイルからQuaternionを変換する
	 * @param arr json配列
	 */
	Quaternion ParseRotation(const nlohmann::json& arr)
	{
		// 配列のサイズと型をチェック
		if (arr.size() != 4 || !arr.is_number())
		{
			return Quaternion::Identity;
		}

		return Quaternion(
			arr[0].get<float>(),
			arr[1].get<float>(),
			arr[2].get<float>(),
			arr[3].get<float>()
		);
	}


	/**
	 * @brief ステージオブジェクトのトランスフォーム情報を読み込む
	 * @param object ステージオブジェクト
	 * @param json トランスフォーム情報のjson
	 */
	void LoadTransform(app::actor::IStageObject* object, const nlohmann::json& json)
	{
		const Vector3 position = ParseVector3(json["position"]);
		const Quaternion rotation = ParseRotation(json["rotation"]);
		const Vector3 scale = ParseVector3(json["scale"]);

		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
		object->UpdateWrapper();
	}


	/**
	 * @brief ステージオブジェクトを初期化する
	 * @param object ステージオブジェクト
	 * @param item jsonデータ
	 */
	void InitializeStageObject(app::actor::IStageObject* object, const nlohmann::json& item)
	{
		const std::string fileName = item["fileName"].get<std::string>();

		object->Init(fileName.c_str());

		LoadTransform(object, item);
		object->StartWrapper();
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

		// ファイルストリームを開く
		std::fstream file(jsonFileName);

		// ファイルが開けなかった場合
		if (!file.is_open()) {
			// 読み込み失敗
			return false;
		}

		nlohmann::json jsonTemp;

		// jsonの読み込みを試す
		try
		{
			file >> jsonTemp;
		}
		// 例外が発生した場合
		catch (...)
		{
			// 読み込み失敗
			return false;
		}
		// 読み込んだjsonを保存
		json = std::move(jsonTemp);
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
				const auto& objectKey = objData[OBJECT_NAME].get<std::string>();

				if (!m_objectMap.empty())
				{
					// マップ内を検索
					auto it = m_objectMap.find(objectKey);
					// 既存のものがあるかチェック
					if (it != m_objectMap.end())
					{
						// スキップ
						continue;
					}
				}

				// ここに来たということは既存のものはないので生成
				auto newObject = std::make_unique<IStageObject>();
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
				const std::string& objectKey = objData[OBJECT_NAME].get<std::string>();
				toBeDeleted.erase(objectKey);
			}

			// 削除対象のキーをもとにオブジェクトを削除
			for (const auto& key : toBeDeleted)
			{
				m_objectMap.erase(key);
			}
		}


		void StageSystem::ReloadTransform(const nlohmann::json& j)
		{
			// 必要なデータが存在するかチェック
			if (!j.contains(STAGE_OBJECT_KEY)) return;
			if (!j[STAGE_OBJECT_KEY].contains(OBJECT_ARRAY_KEY)) return;



			assert(j[STAGE_OBJECT_KEY][OBJECT_ARRAY_KEY].is_array());

			for (auto& obj : m_objectMap)
			{
				assert(obj.second != nullptr);

				LoadTransform(obj.second.get(), j[STAGE_OBJECT_KEY][OBJECT_ARRAY_KEY]);
			}
		}


		StageSystem::StageSystem()
			: m_jsonFileName(JSON_FILE_NAME)
			, m_lastUpdateTime(0)
		{
			/** マップのメモリを確保 */
			m_objectMap.reserve(MAX_OBJECT_NUM);
		}


		void StageSystem::Update()
		{
			for (const auto& obj : m_objectMap)
			{
				obj.second->UpdateWrapper();
			}

#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD

			nlohmann::json json;

			// JSONの読み込みを試す
			if (!TryRoadJsonFile(json, m_jsonFileName, m_lastUpdateTime))
			{
				// 失敗した場合処理しない
				return;
			}


			// 必要なデータが存在するかチェック
			if (!json.contains(STAGE_OBJECT_KEY)) return;
			if (!json[STAGE_OBJECT_KEY].contains(OBJECT_ARRAY_KEY)) return;



			// オブジェクトを削除
			DeleteStageObject(json);

			// オブジェクトを生成
			CreateStageObject(json);

			// トランスフォームの情報をリロード
			ReloadTransform(json);

#endif // ENABLE_OBJECT_LAYOUT_HOTRELOAD
		}


		void StageSystem::Render(RenderContext& rc)
		{
			for (const auto& obj : m_objectMap)
			{
				obj.second->RenderWrapper(rc);
			}
		}


		/** インスタンスを初期化 */
		StageSystem* StageSystem::m_instance = nullptr;
	}
}


