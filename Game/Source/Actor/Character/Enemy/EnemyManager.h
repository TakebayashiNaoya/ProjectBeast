/**
 * @file EnemyManager.h
 * @brief Enemyのマネージャー
 * @author 竹林
 */
#pragma once
#include "json/json.hpp"
#include <vector>

namespace app
{
	namespace actor
	{
		class Enemy;
		class EnemyController;

		class EnemyManager
		{
		public:
			void Update();
			void UpdateModelOnly();
			void Render(RenderContext& rc);

			/**
			 * @brief JSONからエネミーを一括生成
			 * @param json エネミーの配置データ
			 */
			void LoadEnemies(const nlohmann::json& json);

			/**
			 * @brief エネミーの全削除
			 */
			void ClearEnemies();

			/**
			 * @brief エネミーのポインタリストを取得
			 */
			std::vector<Enemy*> GetEnemies() const
			{
				std::vector<Enemy*> list;
				for (const auto& data : m_enemyList)
				{
					list.push_back(data.enemy);
				}
				return list;
			}



		private:
			EnemyManager();
			~EnemyManager();


		private:
			/** エネミーとコントローラーのセット */
			struct EnemyData
			{
				Enemy* enemy = nullptr;
				EnemyController* controller = nullptr;
			};

			/** エネミーのリスト */
			std::vector<EnemyData> m_enemyList;




			//============================================//
			// シングルトン関連
			//============================================//

		public:
			/** インスタンスの生成 */
			static void CreateInstance()
			{
				if (m_instance == nullptr) {
					m_instance = new EnemyManager;
				}
			}
			/** インスタンスの取得 */
			static EnemyManager* GetInstance()
			{
				return m_instance;
			}
			/** インスタンスの破棄 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr) {
					delete m_instance;
					m_instance = nullptr;
				}
			}


		private:
			static EnemyManager* m_instance;
		};
	}// namespace actor
}// namespace app