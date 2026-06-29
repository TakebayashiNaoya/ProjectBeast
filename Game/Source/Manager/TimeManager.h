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
	private:
		/** タイムの最大値（試合時間：秒） */
		static constexpr float MAX_TIME = 180.0f;


	public:
		/**
		 * @brief タイムの最大値を取得
		 */
		inline float GetMaxTime() const { return m_maxTime; }
		/**
		 * @brief タイムの最大値を設定
		 */
		inline void SetMaxTime(const float max) { m_maxTime = max; }

		/**
		 * @brief 現在のタイムを取得
		 */
		inline float GetCurTime() const { return m_currentTime; }

		/**
		 * @brief タイムストップの状態を設定
		 */
		inline void SetIsTimeStop(const bool stop) { m_isTimeStop = stop; }

		/**
		 * @brief タイムアップしているかどうか
		 * @return タイムアップしていれば true
		 */
		inline bool IsTimeUp() const { return m_isTimeUp; }	// 修正：m_isTimeUp を返す（旧: m_currentTime <= m_maxTime）


	public:
		/** タイムの更新 */
		void Update();

		/** タイムのリセット */
		void ResetTime();

		/** ポーズ時に呼ぶ（再開後の最初フレームで経過時間を計算しないようにする） */
		inline void Pause() { m_lastUpdateTime = 0; }


	private:
		TimeManager()
			: m_maxTime(MAX_TIME)
			, m_currentTime(MAX_TIME)
			, m_isTimeStop(false)
			, m_isTimeUp(false)
			, m_lastUpdateTime(0)
			, m_freq(0)
		{
			::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_freq));
		};

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
		/** 前回 Update() を呼んだ時の QPC カウント（0 = 未初期化） */
		LONGLONG m_lastUpdateTime;
		/** QPC 周波数（コンストラクタで1回取得） */
		LONGLONG m_freq;




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