/**
 * @file ScoreManager.h
 * @brief スコアの管理をするクラス
 */
#pragma once
#include <map>
#include <string>


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

		void AddTotalCount() { m_totalCount++; }
		void SubTotalCount() { m_totalCount--; }


		//------------------------------------------------------------
		// ハイスコア（ステージ別・ローカル保存）
		//------------------------------------------------------------

		/**
		 * @brief 最後にプレイしたステージ名を設定する
		 * @details インゲームシーンの開始時に GetStageName() を渡す。
		 *          リザルトでのハイスコア保存先の特定に使う。
		 *          リザルト時点で本インスタンスは破棄済みのため、ハイスコア系はstaticで持つ。
		 * @param stageName ステージ名（"Easy" / "Normal" / "Hard" など）
		 */
		static void SetLastPlayedStage(const std::string& stageName) { s_lastPlayedStage = stageName; }

		/**
		 * @brief 最後にプレイしたステージ名を取得する
		 * @return ステージ名（"Easy" / "Normal" / "Hard" など。未設定なら空文字）
		 */
		static const std::string& GetLastPlayedStage() { return s_lastPlayedStage; }

		/**
		 * @brief 最後にプレイしたステージのハイスコア更新を試みる
		 * @details 既存記録を上回っていればファイルへ保存する。
		 * @param score 今回のスコア
		 * @return 新記録ならtrue
		 */
		static bool TryUpdateHighScore(int score);

		/**
		 * @brief ステージのハイスコアを取得する
		 * @param stageName ステージ名（"Easy" / "Normal" / "Hard"）
		 * @return ハイスコア（記録がなければ0）
		 */
		static int GetHighScore(const std::string& stageName);


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
		/**
		 * @brief ハイスコアをファイルから読み込む（初回アクセス時に一度だけ）
		 */
		static void LoadHighScoresIfNeeded();

		/**
		 * @brief ハイスコアをファイルへ保存する
		 */
		static void SaveHighScores();


	private:
		int m_collectedCount;
		int m_totalCount;

		/** 最後にプレイしたステージ名（ハイスコア保存先の特定用） */
		static std::string s_lastPlayedStage;
		/** ステージ名→ハイスコア */
		static std::map<std::string, int> s_highScores;
		/** ハイスコアをファイルから読み込み済みか */
		static bool s_isHighScoresLoaded;


	private:
		static ScoreManager* m_instance;


	};
}
