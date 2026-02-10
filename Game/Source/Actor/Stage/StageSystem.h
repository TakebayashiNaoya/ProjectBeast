/**
 * @file StageSystem.h
 * @brief ステージ上のオブジェクトを管理するシステム
 * @author 藤谷
 */
#pragma once
#include "json/json.hpp"


#ifdef K2_DEBUG
#define ENABLE_OBJECT_LAYOUT_HOTRELOAD
#endif


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class IStageObject;


		/**
		 *　@brief ステージシステム
		 */
		class StageSystem : public Noncopyable
		{
		public:
			struct StageObjectInfo
			{
				uint8_t objectType;
				uint8_t objectNum;
				std::unique_ptr<IStageObject> object;
			};



			enum class EnObjectType : uint8_t
			{
				enObjectType_Floor = 0,
				enObjectType_Max
			};


		public:
			/**
			 * @brief ステージオブジェクトを生成
			 * @param item JSONのオブジェクト情報
			 */
			void CreateStageObject();


		public:
			StageSystem();
			~StageSystem() = default;


			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);
			/** トランスフォームの情報を読み込み直す */
			void ReloadTransform(const nlohmann::json& j);


		private:
#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			time_t m_lastUpdateTime = 0;
#endif // APP_ENABLE_OBJECT_LAYOUT_HOTRELOAD


			/** jsonファイルネーム */
			std::string m_jsonFileName;
			/** オブジェクトのリスト */
			std::vector<StageObjectInfo*> m_objectList;


		public:
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new StageSystem();
				}
			}


			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


			static StageSystem* GetInstance()
			{
				return m_instance;
			}


		private:
			static StageSystem* m_instance;
		};
	}
}

