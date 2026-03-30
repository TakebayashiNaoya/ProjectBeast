/**
 * @file BattleManager.h
 * @brief バトルの管理をするクラス
 * @author 立山
 */
#pragma once


namespace app
{
	class BattleManager
	{
	public:
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new BattleManager();
			}
		}


		static BattleManager& GetInstance()
		{
			return *m_instance;
		}


		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}

	private:
		BattleManager();
		~BattleManager();


	private:
		static BattleManager* m_instance;
	};
}
