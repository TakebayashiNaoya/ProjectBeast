/**
 * @file AchievementManager.h
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#pragma once
#include "AchievementBase.h"
#include "Json/json.hpp"


namespace app
{
	namespace achievement
	{



		/**
		* @brief アチーブメントの管理クラス
		*/
		class AchievementManager : public Noncopyable
		{
		public:
			/**
			 * @brief 初期化処理
			 */
			void Start();
			/**
			 * @brief 更新処理
			 */
			void Update();
			/**
			 * @brief 描画処理
			 * @note もしかすると必要ないかもしれない
			 */
			void Render(RenderContext& rc);


		public:
			AchievementManager();
			~AchievementManager();


		private:
			/**
			 * @brief アチーブメントを作成する
			 * @todo 未実装なので、この先実装する
			 */
			void CreateAchievement(const nlohmann::json& json);


			using AchieveKey = uint32_t;
			using Achieve = std::unique_ptr<AchievementBase>;


		private:
			/** アチーブメントのマップ */
			std::unordered_map<AchieveKey, Achieve> m_achievementMap;


			/** シングルトン関係 */
		public:
			/**
			 * @brief インスタンスを作成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new AchievementManager();
				}
			}


			/**
			 * @brief インスタンスを取得
			 * @return インスタンス
			 */
			static AchievementManager* GetInstance()
			{
				return m_instance;
			}


			/**
			 * @brief インスタンスを破棄
			 */
			static void DestroyInstance()
			{
				delete m_instance;
				m_instance = nullptr;
			}



		private:
			/** シングルトンインスタンス */
			static AchievementManager* m_instance;
		};
	}
}

