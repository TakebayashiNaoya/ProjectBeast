/**
 * @file EasyInGameScene.h
 * @brief インゲームシーン（イージーステージ）
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"


namespace app
{
	/**
	 * @brief イージーステージ
	 * @detail ステージ固有のパラメータを返す。
	 *         ゲームロジックは InGameSceneBase が担う。
	 */
	class EasyInGameScene : public InGameSceneBase
	{
		appScene(EasyInGameScene);

	protected:
		float GetTimeLimit() const override
		{
			return 120.0f;
		}

		Vector3 GetDaddySpawnPos() const override
		{
			return { 0.0f, 110.0f, -100.0f };
		}

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			return { 50, 50, 0, 0, 0, 3500.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Easy.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return "Assets/parameter/character/enemy/EnemyLayout_Easy.json";
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return "Assets/parameter/stage/whirlpoolPositions_Easy.json";
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Easy.json";
		}

		const char* GetWhirlpoolParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Easy.bin";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Easy.json";
		}

		const char* GetOceanParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Easy.bin";
		}

		const char* GetStageName() const override
		{
			return "Easy";
		}

		const char* GetAchievementJsonPath() const override
		{
			return "Assets/parameter/achievement/AchievementList_Easy.json";
		}

		const char* GetTerrainJsonPath() const override
		{
			return "Assets/parameter/stage/TerrainConfig_Easy.json";
		}
	};
}
