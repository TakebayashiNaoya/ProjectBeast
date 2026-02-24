/**
 * @file NPCControllerManager.h
 * @brief NPCのコントローラーのマネージャー
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{
		class NPCController;
	}

	class NPCControllerManager
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
				m_instance = new NPCControllerManager;
			}
		}


		static NPCControllerManager* GetInstance()
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
		void Register(actor::NPCController* npc);

		/** リストから削除 */
		void UnRegister(actor::NPCController* npc);


	private:
		NPCControllerManager();
		~NPCControllerManager();


	private:
		std::vector<actor::NPCController*> m_npcControllerList;


	private:
		static NPCControllerManager* m_instance;
	};
}
