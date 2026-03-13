/**
 * @file EnemyControllerManager.h
 * @brief Enemyのコントローラーのマネージャー
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{
		class EnemyController;
		class Player;
	}

	class EnemyControllerManager
	{
	public:
		void Update();


	public:
		/**
		 * @brief シングルトンインスタンスを生成
		 * @brief GameSceneのコンストラクタで呼び出す。
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new EnemyControllerManager;
			}
		}


		static EnemyControllerManager* GetInstance()
		{
			return m_instance;
		}


		/**
		 * @brief シングルトンインスタンスを削除
		 * @brief GameSceneのデストラクタで呼び出す
		 */
		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


	public:
		/** Managerに登録 */
		void Register(actor::EnemyController* npc);

		/** リストから削除 */
		void UnRegister(actor::EnemyController* npc);


	public:
		void SetPlayer(actor::Player* player)
		{
			m_player = player;
		}

		actor::Player* GetPlayer() const
		{
			return m_player;
		}


	private:
		EnemyControllerManager();
		~EnemyControllerManager();


	private:
		std::vector<actor::EnemyController*> m_enemyControllerList;


	private:
		static EnemyControllerManager* m_instance;

		actor::Player* m_player = nullptr;
	};
}
