/**
 * @file ScoreManager.cpp
 * @brief スコアの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "ScoreManager.h"
#include "Source/Util/JsonConverter.h"
#include <algorithm>
#include <direct.h>


namespace app
{
	namespace
	{
		/** ハイスコアの保存先。SaveDataフォルダは無ければ作る */
		constexpr const char* HIGH_SCORE_DIR = "SaveData";
		constexpr const char* HIGH_SCORE_PATH = "SaveData/HighScore.json";
	}


	ScoreManager* ScoreManager::m_instance = nullptr;
	std::string ScoreManager::s_lastPlayedStage;
	std::map<std::string, int> ScoreManager::s_highScores;
	bool ScoreManager::s_isHighScoresLoaded = false;


	void ScoreManager::LoadHighScoresIfNeeded()
	{
		if (s_isHighScoresLoaded) return;
		s_isHighScoresLoaded = true;

		nlohmann::json json;
		if (!util::JsonConverter::IsLoadJsonFile(json, HIGH_SCORE_PATH)) return;

		for (auto it = json.begin(); it != json.end(); ++it)
		{
			if (it.value().is_number())
			{
				s_highScores[it.key()] = it.value().get<int>();
			}
		}
	}


	void ScoreManager::SaveHighScores()
	{
		nlohmann::json json;
		for (const auto& pair : s_highScores)
		{
			json[pair.first] = pair.second;
		}

		_mkdir(HIGH_SCORE_DIR);	// 既にあれば失敗するだけなので戻り値は見ない

		FILE* fp = nullptr;
		fopen_s(&fp, HIGH_SCORE_PATH, "w");
		if (fp == nullptr) return;
		const std::string text = json.dump(2);
		fwrite(text.c_str(), 1, text.size(), fp);
		fclose(fp);
	}


	bool ScoreManager::TryUpdateHighScore(int score)
	{
		if (s_lastPlayedStage.empty()) return false;
		LoadHighScoresIfNeeded();

		auto it = s_highScores.find(s_lastPlayedStage);
		if (it != s_highScores.end() && it->second >= score) return false;

		s_highScores[s_lastPlayedStage] = score;
		SaveHighScores();
		return true;
	}


	int ScoreManager::GetHighScore(const std::string& stageName)
	{
		LoadHighScoresIfNeeded();
		auto it = s_highScores.find(stageName);
		return (it != s_highScores.end()) ? it->second : 0;
	}

	ScoreManager::ScoreManager()
		:m_collectedCount(3)
	{

	}


	ScoreManager::~ScoreManager()
	{

	}
}