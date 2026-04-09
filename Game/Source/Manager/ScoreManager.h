/**
 * @file ScoreManager.h
 * @brief スコアの管理をするクラス
 * @author 立山
 */
#pragma once


namespace app
{
	class ScoreManager
	{
	public:


		void SetCollectedCount(int collected) { m_collectedCount = collected; }
		int GetCollectedCount() { return m_collectedCount; }


		void AddCollectedCount() { m_collectedCount++; }
		void SubCollectedCount() { m_collectedCount--; }

		void SetTotalCount(int total) { m_totalCount = total; }
		int GetTotalCount() { return m_totalCount; }

		void SubTotalCount() { m_totalCount--; }


	public:
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new ScoreManager;
			}
		}


		static ScoreManager& GetInstance()
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
		ScoreManager();
		~ScoreManager();

	private:
		int m_collectedCount;
		int m_totalCount;


	private:
		static ScoreManager* m_instance;


	};
}
