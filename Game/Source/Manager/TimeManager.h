/**
 * @file TimeManager.h
 * @brief タイムを管理するクラス
 * @author 立山
 */
#pragma once


namespace app
{
	class TimeManager
	{
	public:

		void Update();


	public:
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new TimeManager();
			}
		}


		static TimeManager& GetInstance()
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


	public:

		void SetMaxTime(float max) { m_maxTime = max; }
		float GetMaxTime() { return m_maxTime; }

		float GetCurTime() { return m_currentTime; }

		void SetTimeStop(bool stop) { m_timeStop = stop; }


	private:
		TimeManager();
		~TimeManager();


	private:
		float m_maxTime;
		float m_currentTime;

		bool m_timeStop;


	private:
		/** シングルトンインスタンス */
		static TimeManager* m_instance;
	};
}
