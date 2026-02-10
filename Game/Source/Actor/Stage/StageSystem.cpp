/**
 * @file Actor.cpp
 * @brief 見た目が存在するオブジェクトの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"
#include "StageSystem.h"
#include <fstream>



 /**
  * @brief　ステージの情報を取得するための関数群
  */
namespace
{
	const char* JSON_FILE_NAME = "Assets/parameter/stage/stageObject.json";


	/**
	 * @brief jsonファイルからVector3を変換する
	 */
	Vector3 ParseVector3(const nlohmann::json& arr)
	{
		return Vector3(
			arr[0].get<float>(),
			arr[1].get<float>(),
			arr[2].get<float>()
		);
	}


	Quaternion ParseRotation(const nlohmann::json& arr)
	{
		Quaternion rot;
		rot.x = arr[0].get<float>();
		rot.y = arr[1].get<float>();
		rot.z = arr[2].get<float>();
		rot.w = arr[3].get<float>();


		return rot;
	}


	void LoadTransform(app::actor::IStageObject* object, const nlohmann::json& item)
	{
		const Vector3 position = ParseVector3(item["position"]);
		const Quaternion rotation = ParseRotation(item["rotation"]);
		const Vector3 scale = ParseVector3(item["scale"]);

		object->SetPosition(position);
		object->SetRotation(rotation);
		object->SetScale(scale);
		object->UpdateWrapper();
	}


	/**
	 *
	 */
	void InitializeStageObject(app::actor::IStageObject* object, const nlohmann::json& item)
	{
		const std::string fileName = item["fileName"].get<std::string>();

		object->Init(fileName.c_str());

		LoadTransform(object, item);
		object->StartWrapper();
	}
}


namespace app
{
	namespace actor
	{
		/** インスタンスを初期化 */
		StageSystem* StageSystem::m_instance = nullptr;


		void StageSystem::CreateStageObject()
		{

			// JSONファイルを開く
			std::ifstream file(m_jsonFileName);
			if (!file.is_open()) {
				return;
			}


			nlohmann::json jsonRoot;
			file >> jsonRoot;

			uint8_t objectNum = 0;

			// オブジェクトの生成
			for (auto& obj : jsonRoot["StageObject"]["object"])
			{

				auto newObject = new StageObjectInfo;
				newObject->objectNum = objectNum;
				newObject->object = std::make_unique<IStageObject>();

				InitializeStageObject(newObject->object.get(), obj);
				m_objectList.push_back(newObject);

				objectNum++;
			}


		}


		StageSystem::StageSystem()
			: m_jsonFileName(JSON_FILE_NAME)
			, m_lastUpdateTime(0)
		{
		}


		void StageSystem::Update()
		{
			for (const auto& obj : m_objectList)
			{
				obj->object->UpdateWrapper();
			}

#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			// ホットリロードチェック
			std::ifstream file(m_jsonFileName);
			if (!file.is_open()) return;

			struct stat st;
			if (stat(m_jsonFileName.c_str(), &st) == 0) {
				if (m_lastUpdateTime != st.st_mtime) {
					m_lastUpdateTime = st.st_mtime;

					nlohmann::json j;

					// JSONの読み込みを試す
					try
					{
						file >> j;
					}
					// 全ての例外をキャッチ
					// ...を条件式に書くことで、全ての例外をキャッチできる
					catch (...)
					{
						return;
					}

					// 必要なデータが存在するかチェック
					if (!j.contains("StageObject")) return;
					if (!j["StageObject"].contains("object")) return;

					ReloadTransform(j);
				}
			}
#endif // ENABLE_OBJECT_LAYOUT_HOTRELOAD
		}


		void StageSystem::Render(RenderContext& rc)
		{
			for (const auto& obj : m_objectList)
			{
				obj->object->RenderWrapper(rc);
			}
		}


		void StageSystem::ReloadTransform(const nlohmann::json& j)
		{
			// 必要なデータが存在するかチェック
			if (!j.contains("StageObject")) return;
			if (!j["StageObject"].contains("object")) return;

			for (auto& obj : m_objectList)
			{
				LoadTransform(obj->object.get(), j["StageObject"]["object"][obj->objectNum]);
			}
		}

	}
}


