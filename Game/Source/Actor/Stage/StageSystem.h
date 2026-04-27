/**
 * @file StageSystem.h
 * @brief ステージ上のオブジェクトを管理するシステム
 * @author 藤谷
 */
#pragma once
#include "IStage.h"
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


		public:
			StageSystem();
			~StageSystem() = default;


			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		private:
#ifdef ENABLE_OBJECT_LAYOUT_HOTRELOAD
			time_t m_lastUpdateTime = 0;
#endif // APP_ENABLE_OBJECT_LAYOUT_HOTRELOAD


			using ObjectKey = std::string;
			using Object = std::unique_ptr<IStageObject>;
			using ObjectMap = std::unordered_map<ObjectKey, Object>;

			/** オブジェクトのマップ */
			ObjectMap m_objectMap;


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



		private:
			/** シングルトンインスタンス */
			static StageSystem* m_instance;
		};
	}
}

