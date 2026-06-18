/**
 * @file StageSystem.h
 * @brief ステージ上のオブジェクトを管理するシステム
 * @author 藤谷
 */
#pragma once
#include "IStage.h"
#include "TerrainObject.h"
#include "json/json.hpp"


#ifdef K2_DEBUG
#define ENABLE_OBJECT_LAYOUT_HOTRELOAD
#endif


namespace app
{
	namespace actor
	{
		/**
		 *　@brief ステージシステム
		 */
		class StageSystem : public Noncopyable
		{
		public:
			/**
			 * @brief ステージオブジェクトを生成
			 */
			void CreateStageObject(const nlohmann::json& json);

			/**
			 * @brief JSON ファイルからステージオブジェクトを生成する
			 * @details デバッグビルドではファイル監視によるホットリロードも有効になる。
			 * @param jsonPath StageObject キーを含む JSON ファイルのパス
			 */
			void LoadStageObjectsFromJson(const std::string& jsonPath);

			/**
			 * @brief ステージオブジェクトを削除
			 */
			void DeleteStageObject(const nlohmann::json& json);

			/**
			 * @briefトランスフォームの情報を読み込み直す
			 */
			void ReloadTransform(const nlohmann::json& j);

			/**
			 * @brief エネミーの巣の情報を読み込む
			 */
			void LoadEnemyNests(const nlohmann::json& json);

			/**
			 * @brief 全ステージオブジェクトのロードが完了しているか
			 */
			bool IsAllLoaded() const;

			/**
			 * @brief ハイトマップ地形を生成する
			 * @details チュートリアルなど地形が必要なシーンから明示的に呼ぶこと。
			 * @param config 地形パラメータ（省略時はデフォルト値）
			 */
			void InitTerrain(const TerrainObject::TerrainConfig& config = {});

			/**
			 * @brief JSON ファイルから地形パラメータを読み込んで地形を生成する
			 * @details デバッグビルドではファイル監視によるホットリロードも有効になる。
			 * @param jsonPath TerrainConfig キーを含む JSON ファイルのパス
			 */
			void InitTerrainFromJson(const std::string& jsonPath);


		public:
			StageSystem();
			~StageSystem() = default;


			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		private:
#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			time_t      m_lastUpdateTime        = 0;
			time_t      m_lastTerrainUpdateTime = 0;
			std::string m_terrainJsonPath;
			std::string m_stageObjectJsonPath;
#endif // ENABLE_OBJECT_LAYOUT_HOTRELOAD


			using ObjectKey = std::string;
			using Object = std::unique_ptr<IStageObject>;
			using ObjectMap = std::unordered_map<ObjectKey, Object>;

			/** オブジェクトのマップ */
			ObjectMap m_objectMap;

			/** ハイトマップ地形 */
			std::unique_ptr<TerrainObject> m_terrain;


		public:
			/** インスタンス生成 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new StageSystem();
				}
			}


			/** インスタンス破棄 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


			/** インスタンス取得 */
			static StageSystem* GetInstance()
			{
				return m_instance;
			}


			/**
			 * @brief ステージオブジェクトのマップのキーを取得
			 * @return ステージオブジェクトのマップのキー
			 */
			const ObjectMap& GetObjectMap() { return m_objectMap; }


		public:
			Vector3 GetObjectPosition(const std::string& key)const;


			Quaternion GetObjectRotation(const std::string& key)const;


			/**
			 * @brief "igloo" で始まるオブジェクトの中から from に最も近い座標を返す
			 * @param from 基準座標（プレイヤーや子ペンギンの現在位置）
			 * @return 最近傍イグルーの座標。該当なしの場合は Vector3::Zero
			 */
			Vector3 GetNearestIglooPosition(const Vector3& from) const;

			/**
			 * @brief "igloo" で始まるオブジェクトの中から from に最も近いオブジェクトの回転を返す
			 * @param from 基準座標（プレイヤーや子ペンギンの現在位置）
			 * @return 最近傍イグルーの回転。該当なしの場合は Quaternion::Identity
			 */
			Quaternion GetNearestIglooRotation(const Vector3& from) const;


			/**
			 * @brief 指定したかまくら（イグルー）を破壊（無効化）する
			 * @param key かまくらのオブジェクトキー
			 */
			void BreakIgloo(const std::string& key);

			/**
			 * @brief from に最も近いイグルーのキー（名前）を返す
			 * @param from 基準座標（シロクマの攻撃着弾点など）
			 * @return 最近傍イグルーのキー。該当なしの場合は空文字列
			 */
			std::string GetNearestIglooKey(const Vector3& from) const;

			/**
			 * @brief "bearHome" で始まるオブジェクトの中から from に最も近い座標を返す
			 * @param from 基準座標（プレイヤーの現在位置）
			 * @return 最近傍シロクマの巣の座標。該当なしの場合は Vector3::Zero
			 */
			Vector3 GetNearestBearNestPosition(const Vector3& from) const;


		private:
			/**
			 * @brief キーが prefix で始まるオブジェクトの中から from に最も近いエントリを返す
			 * @return 見つかった場合はそのイテレータ。なければ m_objectMap.end()
			 */
			ObjectMap::const_iterator FindNearestByPrefix(
				const char* prefix, const Vector3& from) const;

			/** シングルトンインスタンス */
			static StageSystem* m_instance;
		};
	}
}

