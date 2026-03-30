/**
 * @file TimeManager.h
 * @brief タイムを管理するクラス
 * @author 立山、竹林
 */
#pragma once


namespace app
{
	class TimeManager
	{
	public:
		/**
		 * @brief タイムの最大値を取得
		 */
		inline float GetMaxTime() { return m_maxTime; }
		/**
		 * @brief タイムの最大値を設定
		 */
		inline void SetMaxTime(const float max) { m_maxTime = max; }

		/**
		 * @brief 現在のタイムを取得
		 */
		inline float GetCurTime() { return m_currentTime; }

		/**
		 * @brief タイムストップの状態を取得
		 */
		inline void SetIsTimeStop(const bool stop) { m_isTimeStop = stop; }

		/**
		 * @brief タイムアップしているかどうか
		 */
		bool IsTimeUp() const { return m_currentTime <= m_maxTime; }


	public:
		/** タイムの更新 */
		void Update();

		/** タイムのリセット */
		void ResetTime();


	private:
		TimeManager()
			: m_maxTime(FLT_MAX)
			, m_currentTime(FLT_MAX)
			, m_isTimeStop(false)
			, m_isTimeUp(false)
		{};

		~TimeManager() = default;


	private:
		/** タイムの最大値 */
		float m_maxTime;
		/** 現在のタイム */
		float m_currentTime;
		/** タイムストップしているか */
		bool m_isTimeStop;
		/** タイムアップしているか */
		bool m_isTimeUp;




		//============================================//
		// シングルトン関連
		//============================================//

	public:
		/** インスタンスの生成 */
		static void CreateInstance()
		{
			if (m_instance == nullptr) {
				m_instance = new TimeManager();
			}
		}
		/** インスタンスの取得 */
		static TimeManager& GetInstance()
		{
			return *m_instance;
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
		/** シングルトンインスタンス */
		static TimeManager* m_instance;
	};
}
